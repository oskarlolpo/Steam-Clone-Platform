using System;
using System.Drawing;
using System.Drawing.Drawing2D;

namespace CS2WinFormsDemo
{
    public enum WeaponType
    {
        Ak47,
        Deagle,
        Knife
    }

    public class Weapon
    {
        public WeaponType Type { get; set; }
        public string Name { get; set; } = string.Empty;
        public int AmmoClip { get; set; }
        public int MaxClip { get; set; }
        public int AmmoReserve { get; set; }
        public int Damage { get; set; }
        public int HeadshotMultiplier { get; set; } = 4;
        public float FireRate { get; set; }
        public float ReloadTime { get; set; }
        public float TimeSinceLastShot { get; set; } = 10f;
        public bool IsReloading { get; set; } = false;
        public float ReloadTimer { get; set; } = 0f;

        // Visual recoil and muzzle flash
        public float RecoilOffset { get; set; } = 0f;
        public float RecoilPitch { get; set; } = 0f;
        public float MuzzleFlashTimer { get; set; } = 0f;

        // Melee slash animation
        public float SlashTimer { get; set; } = 0f;

        public Weapon(WeaponType type)
        {
            Type = type;
            switch (type)
            {
                case WeaponType.Ak47:
                    Name = "AK-47";
                    MaxClip = 30;
                    AmmoClip = 30;
                    AmmoReserve = 90;
                    Damage = 36;
                    FireRate = 0.10f;
                    ReloadTime = 2.2f;
                    break;
                case WeaponType.Deagle:
                    Name = "Desert Eagle";
                    MaxClip = 7;
                    AmmoClip = 7;
                    AmmoReserve = 35;
                    Damage = 53;
                    FireRate = 0.24f;
                    ReloadTime = 1.9f;
                    break;
                case WeaponType.Knife:
                    Name = "Tactical Knife";
                    MaxClip = 0;
                    AmmoClip = 0;
                    AmmoReserve = 0;
                    Damage = 60;
                    FireRate = 0.35f;
                    ReloadTime = 0f;
                    break;
            }
        }

        public void Update(float dt)
        {
            TimeSinceLastShot += dt;

            if (MuzzleFlashTimer > 0)
            {
                MuzzleFlashTimer -= dt;
                if (MuzzleFlashTimer < 0) MuzzleFlashTimer = 0;
            }

            if (RecoilOffset > 0)
            {
                RecoilOffset -= dt * 180f;
                if (RecoilOffset < 0) RecoilOffset = 0;
            }

            if (RecoilPitch > 0)
            {
                RecoilPitch -= dt * 70f;
                if (RecoilPitch < 0) RecoilPitch = 0;
            }

            if (SlashTimer > 0)
            {
                SlashTimer -= dt * 4.5f;
                if (SlashTimer < 0) SlashTimer = 0;
            }

            if (IsReloading)
            {
                ReloadTimer -= dt;
                if (ReloadTimer <= 0)
                {
                    IsReloading = false;
                    int needed = MaxClip - AmmoClip;
                    int take = Math.Min(needed, AmmoReserve);
                    AmmoClip += take;
                    AmmoReserve -= take;
                }
            }
        }

        public bool CanShoot()
        {
            if (IsReloading) return false;
            if (Type == WeaponType.Knife) return TimeSinceLastShot >= FireRate;
            return TimeSinceLastShot >= FireRate && AmmoClip > 0;
        }

        public bool Fire()
        {
            if (!CanShoot())
            {
                if (!IsReloading && Type != WeaponType.Knife && AmmoClip <= 0 && TimeSinceLastShot >= FireRate)
                {
                    SoundManager.PlayEmpty();
                    TimeSinceLastShot = 0;
                }
                return false;
            }

            TimeSinceLastShot = 0f;
            if (Type == WeaponType.Ak47)
            {
                AmmoClip--;
                MuzzleFlashTimer = 0.055f;
                RecoilOffset = 22f;
                RecoilPitch = 14f;
                SoundManager.PlayShoot();
            }
            else if (Type == WeaponType.Deagle)
            {
                AmmoClip--;
                MuzzleFlashTimer = 0.07f;
                RecoilOffset = 38f;
                RecoilPitch = 24f;
                SoundManager.PlayDeagle();
            }
            else
            {
                SlashTimer = 1.0f;
                SoundManager.PlayHit(false);
            }

            return true;
        }

        public void Reload()
        {
            if (Type == WeaponType.Knife || IsReloading || AmmoClip == MaxClip || AmmoReserve <= 0)
                return;

            IsReloading = true;
            ReloadTimer = ReloadTime;
            SoundManager.PlayReload();
        }

        public void Render(
            Graphics g,
            int screenW,
            int screenH,
            float bobX,
            float bobY,
            float pitch,
            float posZ,
            float swayX,
            float swayY)
        {
            g.SmoothingMode = SmoothingMode.AntiAlias;

            // Screen center / aim point (with recoil kick)
            float aimX = screenW * 0.50f;
            float aimY = screenH * 0.50f - RecoilPitch * 1.2f;

            // Base weapon positioning in first-person 3D perspective
            float baseGunX = screenW * 0.55f + bobX + swayX;
            float baseGunY = screenH * 0.60f + bobY + swayY + RecoilOffset * 0.75f;

            // Pitch reaction: weapon tilts with camera pitch
            // Looking up (pitch > 0) pulls the gun stock down so barrel points up towards crosshair
            // Looking down (pitch < 0) pushes gun stock up so barrel points down towards crosshair
            float pitchInfluence = pitch * 0.44f;
            baseGunY += pitchInfluence;

            // Crouch & jump reaction (inertial vertical lag)
            float heightInfluence = posZ * 38f;
            baseGunY -= heightInfluence;

            if (IsReloading)
            {
                float reloadPhase = (float)Math.Sin((1.0 - (ReloadTimer / ReloadTime)) * Math.PI);
                baseGunY += reloadPhase * 135f;
                baseGunX += reloadPhase * 22f;
            }

            switch (Type)
            {
                case WeaponType.Ak47:
                    RenderAk47FirstPerson(g, baseGunX, baseGunY, screenW, screenH, aimX, aimY, pitch);
                    break;
                case WeaponType.Deagle:
                    RenderDeagleFirstPerson(g, baseGunX, baseGunY, screenW, screenH, aimX, aimY, pitch);
                    break;
                case WeaponType.Knife:
                    RenderKnifeFirstPerson(g, baseGunX, baseGunY, screenW, screenH, aimX, aimY, pitch);
                    break;
            }
        }

        private void RenderAk47FirstPerson(Graphics g, float gunX, float gunY, int screenW, int screenH, float aimX, float aimY, float pitch)
        {
            // AK-47 in realistic first-person 3D perspective tracking crosshair
            float kickY = RecoilPitch * 1.2f;
            float muzX = aimX + 16f + (gunX - screenW * 0.55f) * 0.15f;
            float muzY = aimY + 28f - kickY * 0.5f;

            // Receiver / rear coordinates (near character shoulder/chest)
            float recRightX = gunX + 160f;
            float recRightY = gunY + 105f;
            float recLeftX = gunX + 22f;
            float recLeftY = gunY + 62f;

            // 1. Hands & Arms (Tactical Navy/Grey Glove with reinforced knuckles)
            using (var gloveBase = new SolidBrush(Color.FromArgb(36, 44, 54)))
            using (var gloveDark = new SolidBrush(Color.FromArgb(24, 28, 36)))
            using (var glovePad = new SolidBrush(Color.FromArgb(52, 62, 74)))
            using (var sleeveBrush = new SolidBrush(Color.FromArgb(48, 56, 48))) // Tactical green/grey sleeve
            {
                // Right Arm coming from bottom right
                PointF[] rightArm = {
                    new PointF(screenW * 0.72f + RecoilOffset * 0.4f, screenH),
                    new PointF(screenW * 0.98f, screenH),
                    new PointF(screenW * 0.90f, recRightY + 45f),
                    new PointF(recRightX - 12f, recRightY + 32f)
                };
                g.FillPolygon(sleeveBrush, rightArm);

                // Right Hand wrapping around pistol grip
                g.FillEllipse(gloveBase, recRightX - 44f, recRightY + 8f, 62f, 58f);
                g.FillEllipse(glovePad, recRightX - 32f, recRightY + 18f, 36f, 32f);

                // Left Arm & Hand reaching from bottom left to support handguard
                float leftHandX = muzX + (recLeftX - muzX) * 0.48f;
                float leftHandY = muzY + (recLeftY - muzY) * 0.48f + 24f;

                PointF[] leftArm = {
                    new PointF(screenW * 0.32f, screenH),
                    new PointF(screenW * 0.48f, screenH),
                    new PointF(leftHandX + 44f, leftHandY + 26f),
                    new PointF(leftHandX - 16f, leftHandY + 12f)
                };
                g.FillPolygon(sleeveBrush, leftArm);

                // Left Glove wrapping under wooden handguard
                g.FillEllipse(gloveBase, leftHandX - 22f, leftHandY - 4f, 52f, 42f);
                g.FillEllipse(gloveDark, leftHandX - 14f, leftHandY + 4f, 38f, 26f);
            }

            // 2. Pistol Grip (angled laminate/polymer)
            using (var gripBrush = new SolidBrush(Color.FromArgb(110, 52, 22)))
            using (var gripDark = new SolidBrush(Color.FromArgb(70, 32, 14)))
            {
                PointF[] grip = {
                    new PointF(recRightX - 38f, recRightY + 15f),
                    new PointF(recRightX - 12f, recRightY + 18f),
                    new PointF(recRightX - 4f, recRightY + 75f),
                    new PointF(recRightX - 28f, recRightY + 72f)
                };
                g.FillPolygon(gripBrush, grip);
                g.DrawPolygon(new Pen(gripDark, 2), grip);
            }

            // 3. Curved Bakelite Magazine (Iconic AK orange/amber with ribs)
            using (var magBrush = new SolidBrush(Color.FromArgb(178, 86, 32)))
            using (var magDark = new SolidBrush(Color.FromArgb(122, 54, 18)))
            using (var magBase = new SolidBrush(Color.FromArgb(45, 48, 52)))
            {
                float magTopX = recLeftX + 32f;
                float magTopY = recLeftY + 28f;
                PointF[] magPoints = {
                    new PointF(magTopX, magTopY),
                    new PointF(magTopX + 36f, magTopY + 10f),
                    new PointF(magTopX + 16f, magTopY + 102f),
                    new PointF(magTopX - 26f, magTopY + 86f)
                };
                g.FillPolygon(magBrush, magPoints);
                g.DrawPolygon(new Pen(magDark, 2), magPoints);

                // Horizontal reinforcement ribs on magazine
                using var ribPen = new Pen(magDark, 2.5f);
                for (int r = 1; r <= 4; r++)
                {
                    float factor = r * 0.18f;
                    float rx1 = magPoints[0].X * (1 - factor) + magPoints[3].X * factor;
                    float ry1 = magPoints[0].Y * (1 - factor) + magPoints[3].Y * factor;
                    float rx2 = magPoints[1].X * (1 - factor) + magPoints[2].X * factor;
                    float ry2 = magPoints[1].Y * (1 - factor) + magPoints[2].Y * factor;
                    g.DrawLine(ribPen, rx1 + 3, ry1, rx2 - 3, ry2);
                }

                // Steel base plate
                g.FillRectangle(magBase, magPoints[3].X, magPoints[3].Y - 2, 42f, 8f);
            }

            // 4. Stamped Steel Receiver & Dust Cover
            using (var metalTop = new SolidBrush(Color.FromArgb(68, 74, 82)))
            using (var metalDark = new SolidBrush(Color.FromArgb(30, 33, 38)))
            using (var boltBrush = new SolidBrush(Color.FromArgb(92, 98, 108)))
            {
                // Top receiver dust cover tapering in perspective
                PointF[] topReceiver = {
                    new PointF(muzX + (recLeftX - muzX) * 0.65f, muzY + (recLeftY - muzY) * 0.65f - 10f),
                    new PointF(muzX + (recRightX - muzX) * 0.65f, muzY + (recRightY - muzY) * 0.65f - 8f),
                    new PointF(recRightX, recRightY - 15f),
                    new PointF(recLeftX, recLeftY - 18f)
                };
                g.FillPolygon(metalTop, topReceiver);

                // Right side of receiver
                PointF[] sideReceiver = {
                    new PointF(muzX + (recRightX - muzX) * 0.65f, muzY + (recRightY - muzY) * 0.65f - 8f),
                    new PointF(muzX + (recRightX - muzX) * 0.65f, muzY + (recRightY - muzY) * 0.65f + 26f),
                    new PointF(recRightX, recRightY + 36f),
                    new PointF(recRightX, recRightY - 15f)
                };
                g.FillPolygon(metalDark, sideReceiver);

                // Bolt carrier & charging handle
                float boltX = recRightX - 42f;
                float boltY = recRightY - 5f;
                g.FillRectangle(boltBrush, boltX, boltY, 26f, 10f);
                g.FillEllipse(boltBrush, boltX + 22f, boltY - 2f, 9f, 14f);
            }

            // 5. Wooden Handguard & Gas Tube (Laminated Wood with wood grain)
            using (var woodBase = new SolidBrush(Color.FromArgb(156, 84, 36)))
            using (var woodGrain = new Pen(Color.FromArgb(112, 54, 20), 1.5f))
            using (var woodDark = new SolidBrush(Color.FromArgb(98, 46, 18)))
            {
                float midX1 = muzX + (recLeftX - muzX) * 0.65f;
                float midY1 = muzY + (recLeftY - muzY) * 0.65f;
                float frontX1 = muzX + (recLeftX - muzX) * 0.20f;
                float frontY1 = muzY + (recLeftY - muzY) * 0.20f;

                PointF[] handguard = {
                    new PointF(frontX1, frontY1 - 8f),
                    new PointF(frontX1 + 19f, frontY1 - 6f),
                    new PointF(midX1 + 36f, midY1 - 8f),
                    new PointF(midX1, midY1 - 10f)
                };
                g.FillPolygon(woodBase, handguard);
                g.DrawPolygon(new Pen(woodDark, 2), handguard);

                // Wood grain lines
                g.DrawLine(woodGrain, frontX1 + 4f, frontY1 - 4f, midX1 + 10f, midY1 - 5f);
                g.DrawLine(woodGrain, frontX1 + 12f, frontY1 - 2f, midX1 + 25f, midY1 - 3f);
            }

            // 6. Barrel, Front Sight Post & Muzzle Brake
            using (var barrelBrush = new SolidBrush(Color.FromArgb(32, 35, 40)))
            using (var sightBrush = new SolidBrush(Color.FromArgb(24, 26, 30)))
            {
                // Barrel extending towards muzzle
                PointF[] barrel = {
                    new PointF(muzX - 5f, muzY),
                    new PointF(muzX + 11f, muzY),
                    new PointF(muzX + (recLeftX - muzX) * 0.22f + 15f, muzY + (recLeftY - muzY) * 0.22f),
                    new PointF(muzX + (recLeftX - muzX) * 0.22f - 3f, muzY + (recLeftY - muzY) * 0.22f)
                };
                g.FillPolygon(barrelBrush, barrel);

                // Front Sight Post & Protective Wings
                g.FillRectangle(sightBrush, muzX + 2f, muzY - 16f, 4f, 17f);
                g.DrawArc(new Pen(sightBrush, 2.5f), muzX - 5f, muzY - 20f, 18f, 18f, 180, 180);

                // Slanted AK Muzzle Compensator
                PointF[] comp = {
                    new PointF(muzX - 4f, muzY - 5f),
                    new PointF(muzX + 12f, muzY - 9f),
                    new PointF(muzX + 14f, muzY + 6f),
                    new PointF(muzX - 2f, muzY + 7f)
                };
                g.FillPolygon(barrelBrush, comp);
            }

            // 7. Dynamic Muzzle Flash
            if (MuzzleFlashTimer > 0)
            {
                Render3DMuzzleFlash(g, muzX + 4f, muzY - 2f);
            }
        }

        private void RenderDeagleFirstPerson(Graphics g, float gunX, float gunY, int screenW, int screenH, float aimX, float aimY, float pitch)
        {
            // Desert Eagle in true first-person 3D perspective centered towards aim
            float kickY = RecoilPitch * 1.5f;
            float muzX = aimX + 8f + (gunX - screenW * 0.55f) * 0.12f;
            float muzY = aimY + 42f - kickY * 0.7f;

            float gripX = gunX + 68f;
            float gripY = gunY + 85f;

            // 1. Two-Handed Tactical Gloves
            using (var gloveBrush = new SolidBrush(Color.FromArgb(36, 44, 54)))
            using (var gloveDark = new SolidBrush(Color.FromArgb(24, 28, 36)))
            using (var sleeveBrush = new SolidBrush(Color.FromArgb(48, 56, 48)))
            {
                // Left Support Hand wrapped around bottom of grip
                g.FillEllipse(gloveDark, gripX - 48f, gripY + 22f, 68f, 68f);

                // Right Hand & Arm
                PointF[] rightArm = {
                    new PointF(screenW * 0.66f, screenH),
                    new PointF(screenW * 0.92f, screenH),
                    new PointF(gripX + 48f, gripY + 54f),
                    new PointF(gripX - 12f, gripY + 42f)
                };
                g.FillPolygon(sleeveBrush, rightArm);
                g.FillEllipse(gloveBrush, gripX - 26f, gripY + 12f, 64f, 64f);
            }

            // 2. Black Textured Grip with Eagle Medallion
            using (var blackGrip = new SolidBrush(Color.FromArgb(24, 25, 28)))
            using (var medallion = new SolidBrush(Color.FromArgb(195, 175, 120)))
            {
                PointF[] gripPoly = {
                    new PointF(gripX - 16f, gripY + 10f),
                    new PointF(gripX + 26f, gripY + 10f),
                    new PointF(gripX + 36f, gripY + 92f),
                    new PointF(gripX - 6f, gripY + 86f)
                };
                g.FillPolygon(blackGrip, gripPoly);
                g.FillEllipse(medallion, gripX + 4f, gripY + 40f, 10f, 10f);
            }

            // 3. Polished Chrome / Stainless Steel Slide in 3D Perspective
            using (var chromeTop = new SolidBrush(Color.FromArgb(242, 246, 252)))
            using (var chromeSide = new SolidBrush(Color.FromArgb(195, 202, 212)))
            using (var chromeDark = new SolidBrush(Color.FromArgb(135, 142, 152)))
            using (var blackDetail = new SolidBrush(Color.FromArgb(28, 30, 34)))
            {
                // Top Slide Surface
                PointF[] slideTop = {
                    new PointF(muzX - 10f, muzY - 10f),
                    new PointF(muzX + 18f, muzY - 10f),
                    new PointF(gripX + 36f, gripY - 22f),
                    new PointF(gripX - 16f, gripY - 22f)
                };
                g.FillPolygon(chromeTop, slideTop);

                // Right Slide Wall
                PointF[] slideSide = {
                    new PointF(muzX + 18f, muzY - 10f),
                    new PointF(muzX + 18f, muzY + 16f),
                    new PointF(gripX + 36f, gripY + 16f),
                    new PointF(gripX + 36f, gripY - 22f)
                };
                g.FillPolygon(chromeDark, slideSide);

                // Left Slide Wall
                PointF[] slideLeft = {
                    new PointF(muzX - 10f, muzY - 10f),
                    new PointF(muzX - 10f, muzY + 16f),
                    new PointF(gripX - 16f, gripY + 16f),
                    new PointF(gripX - 16f, gripY - 22f)
                };
                g.FillPolygon(chromeSide, slideLeft);

                // Rear Slide Serrations
                using var serrationPen = new Pen(chromeDark, 2f);
                for (int s = 0; s < 4; s++)
                {
                    float sx = gripX + 16f + s * 5f;
                    g.DrawLine(serrationPen, sx, gripY - 16f, sx, gripY + 8f);
                }

                // Front Sight & Rear Notch
                g.FillRectangle(blackDetail, muzX + 2f, muzY - 19f, 4f, 10f);
                g.FillRectangle(blackDetail, gripX - 9f, gripY - 28f, 5f, 8f);
                g.FillRectangle(blackDetail, gripX + 19f, gripY - 28f, 5f, 8f);

                // Massive .50 AE Muzzle Bore
                g.FillEllipse(blackDetail, muzX + 1f, muzY - 4f, 9f, 9f);
            }

            // 4. Muzzle Flash
            if (MuzzleFlashTimer > 0)
            {
                Render3DMuzzleFlash(g, muzX + 5f, muzY - 1f, 1.35f);
            }
        }

        private void RenderKnifeFirstPerson(Graphics g, float gunX, float gunY, int screenW, int screenH, float aimX, float aimY, float pitch)
        {
            // Tactical Combat Knife in first-person view
            float slashAngle = SlashTimer * 75f;
            float knifeX = gunX + 35f - SlashTimer * 110f;
            float knifeY = gunY + 35f + SlashTimer * 35f;

            // Attack slash motion trail
            if (SlashTimer > 0)
            {
                using var trailPen = new Pen(Color.FromArgb((int)(200 * SlashTimer), 240, 245, 255), 5f);
                using var trailGlow = new Pen(Color.FromArgb((int)(120 * SlashTimer), 255, 60, 60), 12f);
                PointF[] trailPts = {
                    new PointF(gunX + 80f, gunY + 20f),
                    new PointF(aimX + 30f, aimY + 5f),
                    new PointF(aimX - 60f, aimY - 15f)
                };
                g.DrawCurve(trailGlow, trailPts);
                g.DrawCurve(trailPen, trailPts);
            }

            using (var gloveBrush = new SolidBrush(Color.FromArgb(36, 44, 54)))
            using (var gloveDark = new SolidBrush(Color.FromArgb(24, 28, 36)))
            using (var bladeSteel = new SolidBrush(Color.FromArgb(230, 235, 245)))
            using (var bladeDark = new SolidBrush(Color.FromArgb(42, 46, 52)))
            using (var redAccent = new SolidBrush(Color.FromArgb(215, 40, 50)))
            using (var handleBrush = new SolidBrush(Color.FromArgb(26, 28, 32)))
            {
                // Glove / Fist
                g.FillEllipse(gloveBrush, knifeX - 16f, knifeY + 20f, 68f, 58f);
                g.FillEllipse(gloveDark, knifeX - 6f, knifeY + 30f, 45f, 35f);

                // Knife Handle with finger grooves
                PointF[] handle = {
                    new PointF(knifeX - 6f, knifeY + 16f),
                    new PointF(knifeX + 16f, knifeY + 16f),
                    new PointF(knifeX + 18f, knifeY + 68f),
                    new PointF(knifeX - 4f, knifeY + 68f)
                };
                g.FillPolygon(handleBrush, handle);

                // Blade pointing towards crosshair
                PointF[] bladeSpine = {
                    new PointF(knifeX + 5f, knifeY + 16f),
                    new PointF(aimX + 22f - slashAngle, aimY + 12f),
                    new PointF(aimX - slashAngle, aimY - 6f),
                    new PointF(knifeX - 7f, knifeY + 16f)
                };
                g.FillPolygon(bladeDark, bladeSpine);

                // Sharpened Cutting Bevel
                PointF[] bladeEdge = {
                    new PointF(knifeX - 7f, knifeY + 16f),
                    new PointF(aimX - slashAngle, aimY - 6f),
                    new PointF(aimX + 12f - slashAngle, aimY + 4f),
                    new PointF(knifeX + 1f, knifeY + 15f)
                };
                g.FillPolygon(bladeSteel, bladeEdge);

                // Red Tactical Stripe
                PointF[] stripe = {
                    new PointF(knifeX + 2f, knifeY + 15f),
                    new PointF(aimX + 9f - slashAngle, aimY + 5f),
                    new PointF(knifeX - 2f, knifeY + 15f)
                };
                g.FillPolygon(redAccent, stripe);
            }
        }

        private void Render3DMuzzleFlash(Graphics g, float fx, float fy, float scale = 1.0f)
        {
            var rng = new Random();
            float radius = rng.Next(48, 72) * scale;

            using (var outerFire = new SolidBrush(Color.FromArgb(220, 255, 175, 20)))
            using (var innerFire = new SolidBrush(Color.FromArgb(255, 255, 245, 180)))
            using (var glowBrush = new SolidBrush(Color.FromArgb(80, 255, 200, 50)))
            {
                // Soft glow
                g.FillEllipse(glowBrush, fx - radius * 1.5f, fy - radius * 1.5f, radius * 3f, radius * 3f);

                // Star flame
                PointF[] star = new PointF[10];
                for (int i = 0; i < 10; i++)
                {
                    double angle = i * Math.PI / 5.0 + rng.NextDouble() * 0.2;
                    float r = (i % 2 == 0) ? radius : radius * 0.4f;
                    star[i] = new PointF(fx + (float)Math.Cos(angle) * r, fy + (float)Math.Sin(angle) * r);
                }
                g.FillPolygon(outerFire, star);
                g.FillEllipse(innerFire, fx - radius * 0.35f, fy - radius * 0.35f, radius * 0.7f, radius * 0.7f);
            }
        }
    }
}
