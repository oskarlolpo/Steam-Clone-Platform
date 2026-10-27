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

        public void Render(Graphics g, int screenW, int screenH, float bobX, float bobY)
        {
            g.SmoothingMode = SmoothingMode.AntiAlias;

            // Weapon positioning in first-person 3D perspective
            float baseGunX = screenW * 0.55f + bobX;
            float baseGunY = screenH * 0.62f + bobY + RecoilOffset;

            if (IsReloading)
            {
                float reloadPhase = (float)Math.Sin((1.0 - (ReloadTimer / ReloadTime)) * Math.PI);
                baseGunY += reloadPhase * 140f;
                baseGunX += reloadPhase * 20f;
            }

            switch (Type)
            {
                case WeaponType.Ak47:
                    RenderAk47FirstPerson(g, baseGunX, baseGunY, screenW, screenH);
                    break;
                case WeaponType.Deagle:
                    RenderDeagleFirstPerson(g, baseGunX, baseGunY, screenW, screenH);
                    break;
                case WeaponType.Knife:
                    RenderKnifeFirstPerson(g, baseGunX, baseGunY, screenW, screenH);
                    break;
            }
        }

        private void RenderAk47FirstPerson(Graphics g, float gunX, float gunY, int screenW, int screenH)
        {
            // AK-47 in true first-person 3D perspective aiming towards crosshair
            // Center of screen / aim point:
            float targetAimX = screenW * 0.50f;
            float targetAimY = screenH * 0.49f;

            // Recoil kick applied to muzzle
            float kickY = RecoilPitch * 1.2f;

            // Muzzle position (aimed forward towards crosshair)
            float muzX = targetAimX + 15f;
            float muzY = targetAimY + 35f - kickY;

            // Receiver / rear block (close to player right eye/shoulder)
            float recRightX = gunX + 160f;
            float recRightY = gunY + 110f;
            float recLeftX = gunX + 20f;
            float recLeftY = gunY + 70f;

            // 1. Hands & Forearm (Tactical Navy/Grey Glove)
            using (var gloveBrush = new SolidBrush(Color.FromArgb(42, 50, 60)))
            using (var gloveDark = new SolidBrush(Color.FromArgb(28, 34, 42)))
            using (var skinBrush = new SolidBrush(Color.FromArgb(205, 160, 130)))
            {
                // Right Arm coming from bottom-right corner
                PointF[] rightArm = {
                    new PointF(screenW * 0.70f + RecoilOffset * 0.5f, screenH),
                    new PointF(screenW * 0.95f, screenH),
                    new PointF(screenW * 0.88f, recRightY + 40f),
                    new PointF(recRightX - 10f, recRightY + 30f)
                };
                g.FillPolygon(gloveBrush, rightArm);

                // Right Hand gripping pistol grip
                g.FillEllipse(gloveBrush, recRightX - 45f, recRightY + 10f, 65f, 60f);

                // Left Arm & Hand supporting handguard from under-left
                float leftHandX = muzX + (recLeftX - muzX) * 0.45f;
                float leftHandY = muzY + (recLeftY - muzY) * 0.45f + 25f;

                PointF[] leftArm = {
                    new PointF(screenW * 0.35f, screenH),
                    new PointF(screenW * 0.50f, screenH),
                    new PointF(leftHandX + 45f, leftHandY + 25f),
                    new PointF(leftHandX - 15f, leftHandY + 10f)
                };
                g.FillPolygon(gloveBrush, leftArm);

                // Left Glove Fingers wrapping under handguard
                g.FillEllipse(gloveDark, leftHandX - 20f, leftHandY - 5f, 50f, 40f);
            }

            // 2. Curved Magazine (Bakelite Orange/Brown)
            using (var magBrush = new SolidBrush(Color.FromArgb(175, 80, 30)))
            using (var magDark = new SolidBrush(Color.FromArgb(120, 50, 20)))
            {
                float magTopX = recLeftX + 30f;
                float magTopY = recLeftY + 30f;
                PointF[] magPoints = {
                    new PointF(magTopX, magTopY),
                    new PointF(magTopX + 35f, magTopY + 10f),
                    new PointF(magTopX + 15f, magTopY + 95f),
                    new PointF(magTopX - 25f, magTopY + 80f)
                };
                g.FillPolygon(magBrush, magPoints);
                g.DrawPolygon(new Pen(magDark, 2), magPoints);
            }

            // 3. Receiver & Dust Cover (Perspective Quad)
            using (var metalBrush = new SolidBrush(Color.FromArgb(48, 52, 58)))
            using (var metalTop = new SolidBrush(Color.FromArgb(70, 75, 82)))
            using (var metalDark = new SolidBrush(Color.FromArgb(30, 33, 38)))
            {
                // Top receiver cover tapering towards front
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
                    new PointF(muzX + (recRightX - muzX) * 0.65f, muzY + (recRightY - muzY) * 0.65f + 25f),
                    new PointF(recRightX, recRightY + 35f),
                    new PointF(recRightX, recRightY - 15f)
                };
                g.FillPolygon(metalDark, sideReceiver);
            }

            // 4. Wooden Handguard & Gas Tube (Perspective trapezoid)
            using (var woodBrush = new SolidBrush(Color.FromArgb(145, 75, 30)))
            using (var woodDark = new SolidBrush(Color.FromArgb(100, 48, 18)))
            {
                float midX1 = muzX + (recLeftX - muzX) * 0.65f;
                float midY1 = muzY + (recLeftY - muzY) * 0.65f;
                float frontX1 = muzX + (recLeftX - muzX) * 0.20f;
                float frontY1 = muzY + (recLeftY - muzY) * 0.20f;

                PointF[] handguard = {
                    new PointF(frontX1, frontY1 - 8f),
                    new PointF(frontX1 + 18f, frontY1 - 6f),
                    new PointF(midX1 + 35f, midY1 - 8f),
                    new PointF(midX1, midY1 - 10f)
                };
                g.FillPolygon(woodBrush, handguard);
                g.DrawPolygon(new Pen(woodDark, 2), handguard);
            }

            // 5. Barrel & Front Sight Tower (aiming at crosshair)
            using (var barrelBrush = new SolidBrush(Color.FromArgb(35, 38, 42)))
            using (var sightBrush = new SolidBrush(Color.FromArgb(25, 27, 30)))
            {
                // Barrel extending to muzzle
                PointF[] barrel = {
                    new PointF(muzX - 5f, muzY),
                    new PointF(muzX + 10f, muzY),
                    new PointF(muzX + (recLeftX - muzX) * 0.22f + 14f, muzY + (recLeftY - muzY) * 0.22f),
                    new PointF(muzX + (recLeftX - muzX) * 0.22f - 2f, muzY + (recLeftY - muzY) * 0.22f)
                };
                g.FillPolygon(barrelBrush, barrel);

                // Front Sight Post (lined up right below crosshair)
                g.FillRectangle(sightBrush, muzX + 1f, muzY - 14f, 4f, 15f);
                // Sight wings
                g.DrawArc(new Pen(sightBrush, 2), muzX - 5f, muzY - 18f, 16f, 16f, 180, 180);

                // Slanted AK Muzzle Brake
                g.FillRectangle(barrelBrush, muzX - 3f, muzY - 4f, 12f, 10f);
            }

            // 6. Muzzle Flash (3D conical flame erupting forward from muzzle)
            if (MuzzleFlashTimer > 0)
            {
                Render3DMuzzleFlash(g, muzX + 3f, muzY);
            }
        }

        private void RenderDeagleFirstPerson(Graphics g, float gunX, float gunY, int screenW, int screenH)
        {
            // Desert Eagle in true first-person 3D perspective centered towards aim
            float aimX = screenW * 0.50f;
            float aimY = screenH * 0.49f;

            float kickY = RecoilPitch * 1.5f;
            float muzX = aimX + 8f;
            float muzY = aimY + 45f - kickY;

            float gripX = gunX + 70f;
            float gripY = gunY + 90f;

            // Gloved Hands gripping pistol
            using (var gloveBrush = new SolidBrush(Color.FromArgb(42, 50, 60)))
            using (var gloveDark = new SolidBrush(Color.FromArgb(28, 34, 42)))
            {
                // Left Support Hand
                g.FillEllipse(gloveDark, gripX - 45f, gripY + 20f, 65f, 65f);

                // Right Hand & Arm
                PointF[] rightArm = {
                    new PointF(screenW * 0.65f, screenH),
                    new PointF(screenW * 0.90f, screenH),
                    new PointF(gripX + 45f, gripY + 50f),
                    new PointF(gripX - 10f, gripY + 40f)
                };
                g.FillPolygon(gloveBrush, rightArm);
                g.FillEllipse(gloveBrush, gripX - 25f, gripY + 10f, 60f, 60f);
            }

            // Silver / Chrome Slide in perspective
            using (var chrome = new SolidBrush(Color.FromArgb(215, 220, 230)))
            using (var chromeTop = new SolidBrush(Color.FromArgb(240, 245, 255)))
            using (var chromeDark = new SolidBrush(Color.FromArgb(140, 145, 155)))
            using (var blackGrip = new SolidBrush(Color.FromArgb(25, 25, 28)))
            {
                // Grip
                PointF[] gripPoly = {
                    new PointF(gripX - 15f, gripY + 10f),
                    new PointF(gripX + 25f, gripY + 10f),
                    new PointF(gripX + 35f, gripY + 90f),
                    new PointF(gripX - 5f, gripY + 85f)
                };
                g.FillPolygon(blackGrip, gripPoly);

                // Top Slide
                PointF[] slideTop = {
                    new PointF(muzX - 10f, muzY - 10f),
                    new PointF(muzX + 18f, muzY - 10f),
                    new PointF(gripX + 35f, gripY - 20f),
                    new PointF(gripX - 15f, gripY - 20f)
                };
                g.FillPolygon(chromeTop, slideTop);

                // Side Slide
                PointF[] slideSide = {
                    new PointF(muzX + 18f, muzY - 10f),
                    new PointF(muzX + 18f, muzY + 15f),
                    new PointF(gripX + 35f, gripY + 15f),
                    new PointF(gripX + 35f, gripY - 20f)
                };
                g.FillPolygon(chromeDark, slideSide);

                // Left Slide face
                PointF[] slideLeft = {
                    new PointF(muzX - 10f, muzY - 10f),
                    new PointF(muzX - 10f, muzY + 15f),
                    new PointF(gripX - 15f, gripY + 15f),
                    new PointF(gripX - 15f, gripY - 20f)
                };
                g.FillPolygon(chrome, slideLeft);

                // Front Sight & Rear Sight notch
                g.FillRectangle(blackGrip, muzX + 2f, muzY - 18f, 4f, 9f);
                g.FillRectangle(blackGrip, gripX - 8f, gripY - 26f, 5f, 7f);
                g.FillRectangle(blackGrip, gripX + 18f, gripY - 26f, 5f, 7f);
            }

            // Muzzle Flash
            if (MuzzleFlashTimer > 0)
            {
                Render3DMuzzleFlash(g, muzX + 4f, muzY - 2f, 1.25f);
            }
        }

        private void RenderKnifeFirstPerson(Graphics g, float gunX, float gunY, int screenW, int screenH)
        {
            // Tactical Knife in first-person view
            float aimX = screenW * 0.50f;
            float aimY = screenH * 0.52f;

            float slashAngle = SlashTimer * 60f;
            float knifeX = gunX + 40f - SlashTimer * 90f;
            float knifeY = gunY + 40f + SlashTimer * 30f;

            using (var gloveBrush = new SolidBrush(Color.FromArgb(42, 50, 60)))
            using (var bladeBrush = new SolidBrush(Color.FromArgb(220, 225, 235)))
            using (var redAccent = new SolidBrush(Color.FromArgb(200, 35, 45)))
            using (var handleBrush = new SolidBrush(Color.FromArgb(25, 27, 30)))
            {
                // Glove / Fist
                g.FillEllipse(gloveBrush, knifeX - 15f, knifeY + 20f, 65f, 55f);

                // Handle
                g.FillRectangle(handleBrush, knifeX - 5f, knifeY + 15f, 18f, 45f);

                // Blade pointing towards crosshair
                PointF[] blade = {
                    new PointF(knifeX + 4f, knifeY + 15f),
                    new PointF(aimX + 20f - slashAngle, aimY + 10f),
                    new PointF(aimX - slashAngle, aimY - 5f),
                    new PointF(knifeX - 6f, knifeY + 15f)
                };
                g.FillPolygon(bladeBrush, blade);
                g.FillPolygon(redAccent, new PointF[] {
                    new PointF(knifeX + 1f, knifeY + 14f),
                    new PointF(aimX + 8f - slashAngle, aimY + 4f),
                    new PointF(knifeX - 3f, knifeY + 14f)
                });
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
