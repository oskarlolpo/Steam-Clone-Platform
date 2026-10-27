using System;
using System.Drawing;

namespace CS2WinFormsDemo
{
    public static class TextureManager
    {
        public const int TexSize = 64;
        public static uint[][] Textures { get; private set; } = Array.Empty<uint[]>();
        public static uint[] EnemySprite { get; private set; } = Array.Empty<uint>();
        public static uint[] EnemyDeadSprite { get; private set; } = Array.Empty<uint>();

        public static void Initialize()
        {
            Textures = new uint[5][];
            for (int i = 0; i < Textures.Length; i++)
            {
                Textures[i] = new uint[TexSize * TexSize];
            }

            GenerateSandstoneWall(Textures[1]);
            GenerateWoodCrate(Textures[2]);
            GenerateMetalDoors(Textures[3]);
            GenerateBombSiteWall(Textures[4]);

            EnemySprite = new uint[TexSize * TexSize];
            EnemyDeadSprite = new uint[TexSize * TexSize];
            GenerateEnemySprite(EnemySprite);
            GenerateEnemyDeadSprite(EnemyDeadSprite);
        }

        private static uint ToArgb(byte a, byte r, byte g, byte b)
        {
            return ((uint)a << 24) | ((uint)r << 16) | ((uint)g << 8) | b;
        }

        private static void GenerateSandstoneWall(uint[] tex)
        {
            // Dust2 Sandstone brick wall
            var rng = new Random(101);
            int brickHeight = 16;
            int brickWidth = 32;

            for (int y = 0; y < TexSize; y++)
            {
                int row = y / brickHeight;
                bool isMortarY = (y % brickHeight == 0);
                int xOffset = (row % 2 == 1) ? brickWidth / 2 : 0;

                for (int x = 0; x < TexSize; x++)
                {
                    int shiftedX = (x + xOffset) % TexSize;
                    bool isMortarX = (shiftedX % brickWidth == 0);

                    if (isMortarY || isMortarX)
                    {
                        // Mortar color (grey-beige, shadowed)
                        byte mg = (byte)(110 + rng.Next(-10, 10));
                        tex[y * TexSize + x] = ToArgb(255, (byte)(mg + 10), mg, (byte)(mg - 15));
                    }
                    else
                    {
                        // Sandstone brick color (CS Dust 2 tan/yellowish)
                        int noise = rng.Next(-12, 12);
                        byte r = (byte)Math.Clamp(215 + noise, 0, 255);
                        byte g = (byte)Math.Clamp(180 + noise, 0, 255);
                        byte b = (byte)Math.Clamp(125 + noise, 0, 255);
                        tex[y * TexSize + x] = ToArgb(255, r, g, b);
                    }
                }
            }
        }

        private static void GenerateWoodCrate(uint[] tex)
        {
            // CS Site Crate: Wooden planks + dark border + cross brace
            var rng = new Random(202);

            for (int y = 0; y < TexSize; y++)
            {
                for (int x = 0; x < TexSize; x++)
                {
                    bool isBorder = (x < 4 || x >= TexSize - 4 || y < 4 || y >= TexSize - 4);
                    bool isDiag1 = Math.Abs(x - y) <= 2;
                    bool isDiag2 = Math.Abs(x - (TexSize - 1 - y)) <= 2;

                    if (isBorder || isDiag1 || isDiag2)
                    {
                        // Dark reinforced wood/metal frame with rivets
                        int n = rng.Next(-8, 8);
                        byte r = (byte)Math.Clamp(90 + n, 0, 255);
                        byte g = (byte)Math.Clamp(60 + n, 0, 255);
                        byte b = (byte)Math.Clamp(35 + n, 0, 255);
                        // Rivets in corners
                        if ((x == 2 || x == TexSize - 3) && (y == 2 || y == TexSize - 3))
                            tex[y * TexSize + x] = ToArgb(255, 180, 180, 190);
                        else
                            tex[y * TexSize + x] = ToArgb(255, r, g, b);
                    }
                    else
                    {
                        // Wood planks
                        int plankIdx = y / 14;
                        bool plankSeam = (y % 14 == 0);
                        if (plankSeam)
                        {
                            tex[y * TexSize + x] = ToArgb(255, 60, 35, 18);
                        }
                        else
                        {
                            int grain = rng.Next(-10, 10);
                            byte r = (byte)Math.Clamp(165 + grain, 0, 255);
                            byte g = (byte)Math.Clamp(115 + grain, 0, 255);
                            byte b = (byte)Math.Clamp(65 + grain, 0, 255);
                            tex[y * TexSize + x] = ToArgb(255, r, g, b);
                        }
                    }
                }
            }
        }

        private static void GenerateMetalDoors(uint[] tex)
        {
            // CS Dust 2 Double Doors (Metal panels, center vertical crack, bolts)
            var rng = new Random(303);

            for (int y = 0; y < TexSize; y++)
            {
                for (int x = 0; x < TexSize; x++)
                {
                    bool isCenterSeam = (x == 31 || x == 32);
                    bool isPanelBorder = (y % 20 == 0 || x == 2 || x == 61);

                    if (isCenterSeam)
                    {
                        tex[y * TexSize + x] = ToArgb(255, 20, 25, 25);
                    }
                    else if (isPanelBorder)
                    {
                        tex[y * TexSize + x] = ToArgb(255, 70, 80, 75);
                    }
                    else
                    {
                        int n = rng.Next(-8, 8);
                        byte r = (byte)Math.Clamp(105 + n, 0, 255);
                        byte g = (byte)Math.Clamp(125 + n, 0, 255);
                        byte b = (byte)Math.Clamp(120 + n, 0, 255);
                        tex[y * TexSize + x] = ToArgb(255, r, g, b);
                    }
                }
            }
        }

        private static void GenerateBombSiteWall(uint[] tex)
        {
            // Concrete wall with red sprayed "A" logo
            var rng = new Random(404);

            for (int y = 0; y < TexSize; y++)
            {
                for (int x = 0; x < TexSize; x++)
                {
                    int n = rng.Next(-10, 10);
                    byte r = (byte)Math.Clamp(180 + n, 0, 255);
                    byte green = (byte)Math.Clamp(175 + n, 0, 255);
                    byte b = (byte)Math.Clamp(160 + n, 0, 255);
                    tex[y * TexSize + x] = ToArgb(255, r, green, b);
                }
            }

            // Draw a red spray-painted "A"
            using var bmp = new Bitmap(TexSize, TexSize);
            using var gfx = Graphics.FromImage(bmp);
            gfx.Clear(Color.Transparent);
            using var brush = new SolidBrush(Color.FromArgb(200, 220, 30, 30));
            using var font = new Font("Impact", 28, FontStyle.Bold);
            gfx.DrawString("A", font, brush, new PointF(16, 10));

            for (int y = 0; y < TexSize; y++)
            {
                for (int x = 0; x < TexSize; x++)
                {
                    var c = bmp.GetPixel(x, y);
                    if (c.A > 30)
                    {
                        tex[y * TexSize + x] = ToArgb(255, c.R, c.G, c.B);
                    }
                }
            }
        }

        private static void GenerateEnemySprite(uint[] tex)
        {
            // Terrorist Bot Sprite (Phoenix Connexion: balaclava mask, dark sunglasses, brown vest, blue jeans)
            using var bmp = new Bitmap(TexSize, TexSize);
            using var g = Graphics.FromImage(bmp);
            g.Clear(Color.Transparent);

            // Head (Balaclava / Mask)
            using (var headBrush = new SolidBrush(Color.FromArgb(220, 200, 180))) // beige balaclava
            {
                g.FillEllipse(headBrush, 24, 6, 16, 18);
            }
            // Aviator Sunglasses
            using (var glassBrush = new SolidBrush(Color.FromArgb(20, 20, 30)))
            {
                g.FillRectangle(glassBrush, 26, 12, 12, 4);
            }
            // Torso (Brown tactical vest / jacket)
            using (var vestBrush = new SolidBrush(Color.FromArgb(120, 85, 55)))
            {
                g.FillRectangle(vestBrush, 20, 24, 24, 22);
            }
            // Straps / Pouches
            using (var strapBrush = new SolidBrush(Color.FromArgb(40, 40, 40)))
            {
                g.FillRectangle(strapBrush, 22, 28, 4, 18);
                g.FillRectangle(strapBrush, 38, 28, 4, 18);
                g.FillRectangle(strapBrush, 28, 34, 8, 8); // ammo pouch
            }
            // Arms & Weapon in hands
            using (var armBrush = new SolidBrush(Color.FromArgb(100, 70, 45)))
            {
                g.FillRectangle(armBrush, 14, 26, 6, 16);
                g.FillRectangle(armBrush, 44, 26, 6, 16);
            }
            // AK-47 held across chest
            using (var gunBrush = new SolidBrush(Color.FromArgb(30, 30, 35)))
            {
                g.FillRectangle(gunBrush, 18, 34, 28, 5);
            }
            using (var woodBrush = new SolidBrush(Color.FromArgb(139, 69, 19)))
            {
                g.FillRectangle(woodBrush, 38, 34, 8, 4); // stock
            }
            // Legs (Jeans)
            using (var legsBrush = new SolidBrush(Color.FromArgb(60, 75, 95)))
            {
                g.FillRectangle(legsBrush, 22, 46, 8, 16);
                g.FillRectangle(legsBrush, 34, 46, 8, 16);
            }
            // Boots
            using (var bootBrush = new SolidBrush(Color.FromArgb(25, 25, 25)))
            {
                g.FillRectangle(bootBrush, 20, 60, 10, 4);
                g.FillRectangle(bootBrush, 34, 60, 10, 4);
            }

            for (int y = 0; y < TexSize; y++)
            {
                for (int x = 0; x < TexSize; x++)
                {
                    var c = bmp.GetPixel(x, y);
                    tex[y * TexSize + x] = ToArgb(c.A, c.R, c.G, c.B);
                }
            }
        }

        private static void GenerateEnemyDeadSprite(uint[] tex)
        {
            // Fallen enemy sprite (ragdoll on floor)
            using var bmp = new Bitmap(TexSize, TexSize);
            using var g = Graphics.FromImage(bmp);
            g.Clear(Color.Transparent);

            // Blood pool
            using (var bloodBrush = new SolidBrush(Color.FromArgb(180, 130, 0, 0)))
            {
                g.FillEllipse(bloodBrush, 8, 48, 48, 14);
            }
            // Body horizontal on ground
            using (var vestBrush = new SolidBrush(Color.FromArgb(120, 85, 55)))
            {
                g.FillRectangle(vestBrush, 14, 46, 26, 12);
            }
            // Head
            using (var headBrush = new SolidBrush(Color.FromArgb(220, 200, 180)))
            {
                g.FillEllipse(headBrush, 40, 48, 12, 10);
            }
            // Legs
            using (var legsBrush = new SolidBrush(Color.FromArgb(60, 75, 95)))
            {
                g.FillRectangle(legsBrush, 4, 48, 14, 8);
            }

            for (int y = 0; y < TexSize; y++)
            {
                for (int x = 0; x < TexSize; x++)
                {
                    var c = bmp.GetPixel(x, y);
                    tex[y * TexSize + x] = ToArgb(c.A, c.R, c.G, c.B);
                }
            }
        }
    }
}
