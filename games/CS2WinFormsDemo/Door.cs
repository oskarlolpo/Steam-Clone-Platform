using System;

namespace CS2WinFormsDemo
{
    public class Door
    {
        public int X { get; set; }
        public int Y { get; set; }
        public float OpenAmount { get; set; } = 0f; // 0.0 (closed) to 1.0 (fully open)
        public bool IsOpening { get; set; } = false;
        public float AutoCloseTimer { get; set; } = 0f;

        public Door(int x, int y)
        {
            X = x;
            Y = y;
        }

        public void Toggle()
        {
            IsOpening = !IsOpening;
            if (IsOpening) AutoCloseTimer = 5.0f; // Stay open for 5 seconds
        }

        public void Open()
        {
            if (!IsOpening || OpenAmount < 0.95f)
            {
                IsOpening = true;
                AutoCloseTimer = 5.0f;
            }
        }

        public void Update(float dt)
        {
            if (IsOpening)
            {
                if (OpenAmount < 1.0f)
                {
                    OpenAmount += dt * 2.2f;
                    if (OpenAmount > 1.0f) OpenAmount = 1.0f;
                }
                else
                {
                    AutoCloseTimer -= dt;
                    if (AutoCloseTimer <= 0)
                    {
                        IsOpening = false;
                    }
                }
            }
            else
            {
                if (OpenAmount > 0f)
                {
                    OpenAmount -= dt * 2.0f;
                    if (OpenAmount < 0f) OpenAmount = 0f;
                }
            }
        }
    }
}
