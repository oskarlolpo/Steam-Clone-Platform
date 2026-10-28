using System;
using System.Collections.Generic;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Drawing.Imaging;

namespace CS2WinFormsDemo
{
    public class RaycastRenderer
    {
        public int RenderWidth { get; private set; }
        public int RenderHeight { get; private set; }

        private Bitmap _renderBitmap;
        private double[] _zBuffer;

        public RaycastRenderer(int width = 640, int height = 360)
        {
            RenderWidth = width;
            RenderHeight = height;
            _renderBitmap = new Bitmap(RenderWidth, RenderHeight, PixelFormat.Format32bppArgb);
            _zBuffer = new double[RenderWidth];
        }

        public unsafe void RenderScene(
            Player player,
            List<Enemy> enemies,
            Weapon weapon,
            Graphics targetGraphics,
            int targetW,
            int targetH,
            float hitMarkerTimer,
            List<string> killfeed,
            float roundTimerRemaining,
            bool nearDoor)
        {
            var data = _renderBitmap.LockBits(
                new Rectangle(0, 0, RenderWidth, RenderHeight),
                ImageLockMode.WriteOnly,
                PixelFormat.Format32bppArgb);

            uint* scan0 = (uint*)data.Scan0;
            int stride = data.Stride / 4;

            // Horizon with pitch and camera height (jump/crouch)
            int horizon = RenderHeight / 2 + (int)player.Pitch + (int)(player.PosZ * 180.0);
            int clampedHorizon = Math.Clamp(horizon, 0, RenderHeight);

            // 1. Draw Sky / Ceiling
            for (int y = 0; y < clampedHorizon; y++)
            {
                float t = (float)y / Math.Max(1, horizon);
                byte skyR = (byte)Math.Clamp(55 + t * 95, 0, 255);
                byte skyG = (byte)Math.Clamp(120 + t * 75, 0, 255);
                byte skyB = (byte)Math.Clamp(215 + t * 25, 0, 255);
                uint skyCol = 0xFF000000 | ((uint)skyR << 16) | ((uint)skyG << 8) | skyB;

                uint* row = scan0 + y * stride;
                for (int x = 0; x < RenderWidth; x++)
                {
                    row[x] = skyCol;
                }
            }

            // 2. Draw Floor / Ground
            for (int y = clampedHorizon; y < RenderHeight; y++)
            {
                float ft = (float)(y - horizon) / Math.Max(1, RenderHeight - horizon);
                byte floorR = (byte)Math.Clamp(150 - ft * 45, 0, 255);
                byte floorG = (byte)Math.Clamp(125 - ft * 38, 0, 255);
                byte floorB = (byte)Math.Clamp(85 - ft * 28, 0, 255);
                uint floorCol = 0xFF000000 | ((uint)floorR << 16) | ((uint)floorG << 8) | floorB;

                uint* row = scan0 + y * stride;
                for (int x = 0; x < RenderWidth; x++)
                {
                    row[x] = floorCol;
                }
            }

            // 3. Raycast Walls & Sliding Doors (DDA)
            for (int x = 0; x < RenderWidth; x++)
            {
                double cameraX = 2.0 * x / RenderWidth - 1.0;
                double rayDirX = player.DirX + player.PlaneX * cameraX;
                double rayDirY = player.DirY + player.PlaneY * cameraX;

                int mapX = (int)Math.Floor(player.X);
                int mapY = (int)Math.Floor(player.Y);

                double deltaDistX = (rayDirX == 0) ? 1e30 : Math.Abs(1.0 / rayDirX);
                double deltaDistY = (rayDirY == 0) ? 1e30 : Math.Abs(1.0 / rayDirY);

                double sideDistX;
                double sideDistY;
                int stepX;
                int stepY;

                int hit = 0;
                int side = 0; // 0 for X, 1 for Y
                double perpWallDist = 0;
                int texX = 0;

                if (rayDirX < 0)
                {
                    stepX = -1;
                    sideDistX = (player.X - mapX) * deltaDistX;
                }
                else
                {
                    stepX = 1;
                    sideDistX = (mapX + 1.0 - player.X) * deltaDistX;
                }

                if (rayDirY < 0)
                {
                    stepY = -1;
                    sideDistY = (player.Y - mapY) * deltaDistY;
                }
                else
                {
                    stepY = 1;
                    sideDistY = (mapY + 1.0 - player.Y) * deltaDistY;
                }

                while (hit == 0)
                {
                    if (sideDistX < sideDistY)
                    {
                        sideDistX += deltaDistX;
                        mapX += stepX;
                        side = 0;
                    }
                    else
                    {
                        sideDistY += deltaDistY;
                        mapY += stepY;
                        side = 1;
                    }

                    int wallVal = GameMap.GetWall(mapX, mapY);
                    if (wallVal > 0)
                    {
                        if (wallVal == 3) // Metal Door
                        {
                            var door = GameMap.GetDoor(mapX, mapY);
                            if (door != null)
                            {
                                double dist = (side == 0) ? (sideDistX - deltaDistX) : (sideDistY - deltaDistY);
                                double hitCoord = (side == 0) ? (player.Y + dist * rayDirY) : (player.X + dist * rayDirX);
                                hitCoord -= Math.Floor(hitCoord);

                                // If ray hits open gap of sliding door, pass through!
                                if (hitCoord < door.OpenAmount)
                                {
                                    continue;
                                }

                                perpWallDist = dist;
                                hit = wallVal;
                                double doorTexOffset = hitCoord - door.OpenAmount;
                                texX = (int)(doorTexOffset * TextureManager.TexSize);
                                break;
                            }
                        }

                        hit = wallVal;
                        if (side == 0)
                            perpWallDist = (sideDistX - deltaDistX);
                        else
                            perpWallDist = (sideDistY - deltaDistY);

                        double wallCoord;
                        if (side == 0)
                            wallCoord = player.Y + perpWallDist * rayDirY;
                        else
                            wallCoord = player.X + perpWallDist * rayDirX;
                        wallCoord -= Math.Floor(wallCoord);

                        texX = (int)(wallCoord * TextureManager.TexSize);
                        if (side == 0 && rayDirX > 0) texX = TextureManager.TexSize - texX - 1;
                        if (side == 1 && rayDirY < 0) texX = TextureManager.TexSize - texX - 1;
                        break;
                    }
                }

                if (perpWallDist <= 0.01) perpWallDist = 0.01;
                _zBuffer[x] = perpWallDist;

                int lineHeight = (int)(RenderHeight / perpWallDist);
                int drawStart = -lineHeight / 2 + horizon;
                int drawEnd = lineHeight / 2 + horizon;

                int clampStart = Math.Clamp(drawStart, 0, RenderHeight);
                int clampEnd = Math.Clamp(drawEnd, 0, RenderHeight);

                int texNum = Math.Clamp(hit, 1, TextureManager.Textures.Length - 1);
                uint[] tex = TextureManager.Textures[texNum];

                float distFactor = (float)Math.Clamp(1.0 / (1.0 + perpWallDist * 0.12), 0.2, 1.0);
                if (side == 1) distFactor *= 0.75f;

                double step = 1.0 * TextureManager.TexSize / lineHeight;
                double texPos = (clampStart - horizon + lineHeight / 2) * step;

                for (int y = clampStart; y < clampEnd; y++)
                {
                    int texY = (int)texPos & (TextureManager.TexSize - 1);
                    texPos += step;

                    uint color = tex[texY * TextureManager.TexSize + (texX & (TextureManager.TexSize - 1))];
                    byte r = (byte)(((color >> 16) & 0xFF) * distFactor);
                    byte g = (byte)(((color >> 8) & 0xFF) * distFactor);
                    byte b = (byte)((color & 0xFF) * distFactor);

                    scan0[y * stride + x] = 0xFF000000 | ((uint)r << 16) | ((uint)g << 8) | b;
                }
            }

            // 4. Render Billboards (Enemies/Bots) with pitch and height
            RenderSprites(player, enemies, scan0, stride, horizon);

            _renderBitmap.UnlockBits(data);

            // 5. Blit 3D Frame to target window
            targetGraphics.InterpolationMode = InterpolationMode.NearestNeighbor;
            targetGraphics.PixelOffsetMode = PixelOffsetMode.Half;
            targetGraphics.DrawImage(_renderBitmap, 0, 0, targetW, targetH);

            // 6. Draw 3D Viewmodel (Gun & Hands in first-person perspective)
            weapon.Render(
                targetGraphics,
                targetW,
                targetH,
                player.BobAmountX,
                player.BobAmountY,
                player.Pitch,
                (float)player.PosZ,
                player.MouseSwayX,
                player.MouseSwayY);

            // 7. Draw HUD (Crosshair, Radar, Health, Ammo, Timer, Door hints)
            DrawTacticalHud(targetGraphics, targetW, targetH, player, weapon, enemies, hitMarkerTimer, killfeed, roundTimerRemaining, nearDoor);
        }

        private unsafe void RenderSprites(Player player, List<Enemy> enemies, uint* scan0, int stride, int horizon)
        {
            enemies.Sort((a, b) => b.DistanceToPlayer.CompareTo(a.DistanceToPlayer));

            for (int i = 0; i < enemies.Count; i++)
            {
                var enemy = enemies[i];

                double spriteX = enemy.X - player.X;
                double spriteY = enemy.Y - player.Y;

                double invDet = 1.0 / (player.PlaneX * player.DirY - player.DirX * player.PlaneY);
                double transformX = invDet * (player.DirY * spriteX - player.DirX * spriteY);
                double transformY = invDet * (-player.PlaneY * spriteX + player.PlaneX * spriteY);

                if (transformY <= 0.2) continue;

                int spriteScreenX = (int)((RenderWidth / 2) * (1 + transformX / transformY));
                int spriteHeight = Math.Abs((int)(RenderHeight / transformY));
                int spriteWidth = spriteHeight;

                // Adjust sprite Y based on horizon (pitch + jump/crouch)
                int spriteScreenY = horizon;
                int drawStartY = -spriteHeight / 2 + spriteScreenY;
                if (drawStartY < 0) drawStartY = 0;
                int drawEndY = spriteHeight / 2 + spriteScreenY;
                if (drawEndY >= RenderHeight) drawEndY = RenderHeight - 1;

                int drawStartX = -spriteWidth / 2 + spriteScreenX;
                if (drawStartX < 0) drawStartX = 0;
                int drawEndX = spriteWidth / 2 + spriteScreenX;
                if (drawEndX >= RenderWidth) drawEndX = RenderWidth - 1;

                uint[] spr = enemy.IsDead ? TextureManager.EnemyDeadSprite : TextureManager.EnemySprite;
                float distFactor = (float)Math.Clamp(1.0 / (1.0 + transformY * 0.12), 0.25, 1.0);
                bool hitFlash = enemy.HitFlash > 0.05f;

                for (int stripe = drawStartX; stripe < drawEndX; stripe++)
                {
                    int texX = (int)((stripe - (-spriteWidth / 2 + spriteScreenX)) * TextureManager.TexSize / spriteWidth);

                    if (transformY > 0 && stripe > 0 && stripe < RenderWidth && transformY < _zBuffer[stripe])
                    {
                        for (int y = drawStartY; y < drawEndY; y++)
                        {
                            int d = (y - spriteScreenY) * 256 + spriteHeight * 128;
                            int texY = ((d * TextureManager.TexSize) / spriteHeight) / 256;
                            if (texY < 0 || texY >= TextureManager.TexSize || texX < 0 || texX >= TextureManager.TexSize)
                                continue;

                            uint color = spr[texY * TextureManager.TexSize + texX];
                            byte alpha = (byte)((color >> 24) & 0xFF);
                            if (alpha > 40)
                            {
                                byte r = (byte)(((color >> 16) & 0xFF) * distFactor);
                                byte g = (byte)(((color >> 8) & 0xFF) * distFactor);
                                byte b = (byte)((color & 0xFF) * distFactor);

                                if (hitFlash)
                                {
                                    r = 255;
                                    g = (byte)(g * 0.2f);
                                    b = (byte)(b * 0.2f);
                                }

                                scan0[y * stride + stripe] = 0xFF000000 | ((uint)r << 16) | ((uint)g << 8) | b;
                            }
                        }
                    }
                }
            }
        }

        private void DrawTacticalHud(
            Graphics g,
            int w,
            int h,
            Player player,
            Weapon weapon,
            List<Enemy> enemies,
            float hitMarkerTimer,
            List<string> killfeed,
            float roundTimerRemaining,
            bool nearDoor)
        {
            float cx = w / 2f;
            float cy = h / 2f - weapon.RecoilPitch * 1.2f;

            // Dynamic Crosshair
            float spread = 4f + (player.MoveForward || player.MoveBackward || player.StrafeLeft || player.StrafeRight ? 8f : 0f);
            if (player.PosZ > player.CrouchOffset + 0.05) spread += 12f; // Jump inaccuracy
            if (player.IsCrouching) spread *= 0.65f; // Crouch precision
            if (weapon.RecoilOffset > 0) spread += weapon.RecoilOffset * 0.35f;

            using (var chPen = new Pen(Color.FromArgb(235, 0, 255, 65), 2))
            using (var chDot = new SolidBrush(Color.FromArgb(235, 0, 255, 65)))
            {
                g.FillRectangle(chDot, cx - 1, cy - 1, 2, 2);
                g.DrawLine(chPen, cx - spread - 8, cy, cx - spread, cy);
                g.DrawLine(chPen, cx + spread, cy, cx + spread + 8, cy);
                g.DrawLine(chPen, cx, cy - spread - 8, cx, cy - spread);
                g.DrawLine(chPen, cx, cy + spread, cx, cy + spread + 8);
            }

            // Hitmarker (Red X)
            if (hitMarkerTimer > 0)
            {
                using var hmPen = new Pen(Color.FromArgb((int)(255 * (hitMarkerTimer / 0.25f)), 255, 40, 40), 2);
                g.DrawLine(hmPen, cx - 7, cy - 7, cx + 7, cy + 7);
                g.DrawLine(hmPen, cx - 7, cy + 7, cx + 7, cy - 7);
            }

            // Bottom-Left: Health & Armor & Crouch Indicator
            using (var hudFont = new Font("Arial", 18, FontStyle.Bold))
            using (var subFont = new Font("Arial", 11, FontStyle.Bold))
            using (var bgBrush = new SolidBrush(Color.FromArgb(170, 15, 20, 25)))
            using (var textBrush = new SolidBrush(Color.White))
            using (var greenBrush = new SolidBrush(Color.FromArgb(80, 220, 100)))
            {
                // Health
                g.FillRectangle(bgBrush, 30, h - 65, 130, 42);
                g.DrawString("+", hudFont, greenBrush, 36, h - 60);
                g.DrawString($"{player.Health}", hudFont, textBrush, 62, h - 60);

                // Armor
                g.FillRectangle(bgBrush, 170, h - 65, 120, 42);
                g.DrawString("🛡", hudFont, textBrush, 175, h - 60);
                g.DrawString($"{player.Armor}", hudFont, textBrush, 210, h - 60);

                // Crouch indicator
                if (player.IsCrouching)
                {
                    g.FillRectangle(bgBrush, 300, h - 65, 90, 42);
                    g.DrawString("DUCK", subFont, greenBrush, 315, h - 52);
                }

                // Bottom-Right: Ammo & Weapon name
                g.FillRectangle(bgBrush, w - 210, h - 65, 180, 42);
                if (weapon.Type != WeaponType.Knife)
                {
                    g.DrawString($"{weapon.AmmoClip} / {weapon.AmmoReserve}", hudFont, textBrush, w - 195, h - 60);
                }
                else
                {
                    g.DrawString("MELEE", hudFont, textBrush, w - 170, h - 60);
                }
                g.DrawString(weapon.Name.ToUpper(), subFont, greenBrush, w - 195, h - 85);

                if (weapon.IsReloading)
                {
                    g.DrawString("RELOADING...", subFont, new SolidBrush(Color.Orange), w / 2 - 55, cy + 35);
                }
            }

            // Top-Center Match Bar with Ticking Timer
            int minutes = (int)roundTimerRemaining / 60;
            int seconds = (int)roundTimerRemaining % 60;
            string timeStr = $"{minutes:D2}:{seconds:D2}";

            using (var barBg = new SolidBrush(Color.FromArgb(190, 20, 25, 30)))
            using (var ctBrush = new SolidBrush(Color.FromArgb(100, 160, 230)))
            using (var tBrush = new SolidBrush(Color.FromArgb(230, 180, 90)))
            using (var timeFont = new Font("Arial", 15, FontStyle.Bold))
            {
                g.FillRectangle(barBg, w / 2 - 110, 15, 220, 38);
                g.DrawString("CT 4", timeFont, ctBrush, w / 2 - 95, 22);
                g.DrawString(timeStr, timeFont, Brushes.White, w / 2 - 25, 22);
                g.DrawString("3 T", timeFont, tBrush, w / 2 + 45, 22);
            }

            // Near Door Prompt
            if (nearDoor)
            {
                using var promptBg = new SolidBrush(Color.FromArgb(210, 10, 15, 20));
                using var promptFont = new Font("Segoe UI", 12, FontStyle.Bold);
                using var yellowBrush = new SolidBrush(Color.FromArgb(255, 215, 60));
                string doorHint = "[E] Открыть дверь";
                var sz = g.MeasureString(doorHint, promptFont);
                g.FillRectangle(promptBg, w / 2 - sz.Width / 2 - 12, cy + 60, sz.Width + 24, sz.Height + 10);
                g.DrawString(doorHint, promptFont, yellowBrush, w / 2 - sz.Width / 2, cy + 65);
            }

            // Radar
            DrawRadar(g, player, enemies);

            // Killfeed
            DrawKillfeed(g, w, killfeed);
        }

        private void DrawRadar(Graphics g, Player player, List<Enemy> enemies)
        {
            int rx = 25, ry = 25, rSize = 110;
            int rCenter = rx + rSize / 2;

            using (var rBg = new SolidBrush(Color.FromArgb(190, 15, 20, 25)))
            using (var rBorder = new Pen(Color.FromArgb(100, 80, 120, 90), 2))
            {
                g.FillEllipse(rBg, rx, ry, rSize, rSize);
                g.DrawEllipse(rBorder, rx, ry, rSize, rSize);
            }

            using (var wallBrush = new SolidBrush(Color.FromArgb(120, 100, 110, 120)))
            {
                int pX = (int)player.X;
                int pY = (int)player.Y;
                float scale = 6.5f;

                for (int y = pY - 7; y <= pY + 7; y++)
                {
                    for (int x = pX - 7; x <= pX + 7; x++)
                    {
                        if (GameMap.IsWall(x, y))
                        {
                            float dx = (float)(x - player.X) * scale;
                            float dy = (float)(y - player.Y) * scale;
                            if (dx * dx + dy * dy < (rSize / 2f - 4) * (rSize / 2f - 4))
                            {
                                g.FillRectangle(wallBrush, rCenter + dx - 2, rCenter + dy - 2, 5, 5);
                            }
                        }
                    }
                }
            }

            using (var enemyBlip = new SolidBrush(Color.FromArgb(235, 45, 45)))
            {
                float scale = 6.5f;
                foreach (var enemy in enemies)
                {
                    if (enemy.IsDead) continue;
                    float dx = (float)(enemy.X - player.X) * scale;
                    float dy = (float)(enemy.Y - player.Y) * scale;
                    if (dx * dx + dy * dy < (rSize / 2f - 6) * (rSize / 2f - 6))
                    {
                        g.FillEllipse(enemyBlip, rCenter + dx - 3, rCenter + dy - 3, 6, 6);
                    }
                }
            }

            using (var pBrush = new SolidBrush(Color.FromArgb(80, 255, 120)))
            {
                float angle = (float)Math.Atan2(player.DirY, player.DirX);
                PointF p1 = new PointF(rCenter + (float)Math.Cos(angle) * 7f, rCenter + (float)Math.Sin(angle) * 7f);
                PointF p2 = new PointF(rCenter + (float)Math.Cos(angle + 2.5) * 5f, rCenter + (float)Math.Sin(angle + 2.5) * 5f);
                PointF p3 = new PointF(rCenter + (float)Math.Cos(angle - 2.5) * 5f, rCenter + (float)Math.Sin(angle - 2.5) * 5f);
                g.FillPolygon(pBrush, new PointF[] { p1, p2, p3 });
            }
        }

        private void DrawKillfeed(Graphics g, int w, List<string> killfeed)
        {
            if (killfeed == null || killfeed.Count == 0) return;
            using var kfBg = new SolidBrush(Color.FromArgb(170, 20, 22, 28));
            using var kfFont = new Font("Arial", 10, FontStyle.Bold);
            using var ctBrush = new SolidBrush(Color.FromArgb(110, 180, 245));

            int startY = 20;
            for (int i = 0; i < killfeed.Count; i++)
            {
                string text = killfeed[i];
                var size = g.MeasureString(text, kfFont);
                float x = w - size.Width - 30;
                float y = startY + i * 26;

                g.FillRectangle(kfBg, x - 6, y - 2, size.Width + 12, size.Height + 4);
                g.DrawString(text, kfFont, ctBrush, x, y);
            }
        }
    }
}
