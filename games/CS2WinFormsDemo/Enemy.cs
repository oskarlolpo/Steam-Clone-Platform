using System;

namespace CS2WinFormsDemo
{
    public class Enemy
    {
        public double X { get; set; }
        public double Y { get; set; }
        public int Health { get; set; } = 100;
        public bool IsDead => Health <= 0;
        public float HitFlash { get; set; } = 0f;
        public float DeathProgress { get; set; } = 0f;
        public double DistanceToPlayer { get; set; } = 0;

        public Enemy(double x, double y)
        {
            X = x;
            Y = y;
        }

        public void Update(float dt, double playerX, double playerY)
        {
            if (HitFlash > 0)
            {
                HitFlash -= dt * 4f;
                if (HitFlash < 0) HitFlash = 0;
            }

            if (IsDead)
            {
                if (DeathProgress < 1.0f)
                {
                    DeathProgress += dt * 3.0f;
                    if (DeathProgress > 1.0f) DeathProgress = 1.0f;
                }
            }
            else
            {
                // Subtle breathing / idle sway
                DistanceToPlayer = Math.Sqrt((playerX - X) * (playerX - X) + (playerY - Y) * (playerY - Y));
            }
        }

        public bool TakeDamage(int damage)
        {
            if (IsDead) return false;
            Health -= damage;
            HitFlash = 1.0f;
            if (Health <= 0)
            {
                Health = 0;
                return true; // Killed!
            }
            return false;
        }
    }
}
