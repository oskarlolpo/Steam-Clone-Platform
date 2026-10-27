using System;

namespace CS2WinFormsDemo
{
    public class Player
    {
        public double X { get; set; } = 3.5;
        public double Y { get; set; } = 3.5;

        // Camera direction
        public double DirX { get; set; } = 1.0;
        public double DirY { get; set; } = 0.0;

        // Camera plane (~66 FOV)
        public double PlaneX { get; set; } = 0.0;
        public double PlaneY { get; set; } = 0.66;

        // Vertical look (Pitch) in screen pixels
        public float Pitch { get; set; } = 0f;

        // Vertical position for jump & crouch (0 = normal eye height)
        public double PosZ { get; set; } = 0.0;
        public double ZVelocity { get; set; } = 0.0;
        public bool IsCrouching { get; set; } = false;
        public double CrouchOffset { get; set; } = 0.0;

        public int Health { get; set; } = 100;
        public int Armor { get; set; } = 100;

        public bool MoveForward { get; set; }
        public bool MoveBackward { get; set; }
        public bool StrafeLeft { get; set; }
        public bool StrafeRight { get; set; }
        public bool IsRunning { get; set; } = true;

        public float BobCycle { get; set; } = 0f;
        public float BobAmountX { get; set; } = 0f;
        public float BobAmountY { get; set; } = 0f;

        private const double Radius = 0.25;

        public void Rotate(double angle)
        {
            double oldDirX = DirX;
            DirX = DirX * Math.Cos(angle) - DirY * Math.Sin(angle);
            DirY = oldDirX * Math.Sin(angle) + DirY * Math.Cos(angle);

            double oldPlaneX = PlaneX;
            PlaneX = PlaneX * Math.Cos(angle) - PlaneY * Math.Sin(angle);
            PlaneY = oldPlaneX * Math.Sin(angle) + PlaneY * Math.Cos(angle);
        }

        public void LookVertical(float dPitch)
        {
            Pitch += dPitch;
            Pitch = Math.Clamp(Pitch, -160f, 160f);
        }

        public void Jump()
        {
            // Only jump if on ground
            if (PosZ <= CrouchOffset + 0.02)
            {
                ZVelocity = 0.65;
                SoundManager.PlayJump();
            }
        }

        public void Update(float dt)
        {
            // Crouch height transition
            double targetCrouch = IsCrouching ? -0.38 : 0.0;
            CrouchOffset += (targetCrouch - CrouchOffset) * Math.Min(1.0, dt * 14.0);

            // Gravity & Vertical physics
            ZVelocity -= 2.4 * dt;
            PosZ += ZVelocity * dt;

            if (PosZ <= CrouchOffset)
            {
                PosZ = CrouchOffset;
                ZVelocity = 0;
            }

            // Movement speed: base 4.8, shift walk 2.5, crouch walk 1.8
            double baseSpeed = IsCrouching ? 1.8 : (IsRunning ? 4.8 : 2.5);
            double speed = baseSpeed * dt;
            bool isMoving = false;

            double moveX = 0;
            double moveY = 0;

            if (MoveForward)
            {
                moveX += DirX * speed;
                moveY += DirY * speed;
                isMoving = true;
            }
            if (MoveBackward)
            {
                moveX -= DirX * speed;
                moveY -= DirY * speed;
                isMoving = true;
            }
            if (StrafeRight)
            {
                moveX += PlaneX * speed;
                moveY += PlaneY * speed;
                isMoving = true;
            }
            if (StrafeLeft)
            {
                moveX -= PlaneX * speed;
                moveY -= PlaneY * speed;
                isMoving = true;
            }

            // Collision detection with sliding
            if (moveX != 0)
            {
                double checkX = X + moveX + Math.Sign(moveX) * Radius;
                if (!GameMap.IsWall(checkX, Y))
                {
                    X += moveX;
                }
            }
            if (moveY != 0)
            {
                double checkY = Y + moveY + Math.Sign(moveY) * Radius;
                if (!GameMap.IsWall(X, checkY))
                {
                    Y += moveY;
                }
            }

            // Head Bobbing
            if (isMoving && PosZ <= CrouchOffset + 0.05)
            {
                BobCycle += dt * (IsCrouching ? 6f : (IsRunning ? 10f : 7f));
                BobAmountX = (float)Math.Cos(BobCycle * 0.5f) * 6f;
                BobAmountY = (float)Math.Abs(Math.Sin(BobCycle)) * 8f;
            }
            else
            {
                BobAmountX = (float)Math.Cos(BobCycle * 0.5f) * 1.5f;
                BobAmountY = (float)Math.Sin(BobCycle * 0.5f) * 1.5f;
                BobCycle += dt * 2f;
            }
        }
    }
}
