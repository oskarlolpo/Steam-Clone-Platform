using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.Windows.Forms;

namespace CS2WinFormsDemo
{
    public partial class Form1 : Form
    {
        private Player _player;
        private List<Enemy> _enemies;
        private Weapon[] _weapons;
        private int _currentWeaponIndex = 0;
        private RaycastRenderer _renderer;

        private Stopwatch _stopwatch;
        private long _lastTicks;
        private System.Windows.Forms.Timer _gameLoopTimer;

        private bool _isMouseCaptured = false;
        private bool _isLeftMouseDown = false;
        private bool _rotateLeftKey = false;
        private bool _rotateRightKey = false;
        private bool _lookUpKey = false;
        private bool _lookDownKey = false;
        private float _mouseSensitivity = 0.0035f;

        private float _hitMarkerTimer = 0f;
        private List<string> _killfeed = new List<string>();
        private List<float> _killfeedTimers = new List<float>();

        private float _roundTimeRemaining = 115f; // 1 min 55 sec, counts down!
        private bool _nearDoor = false;
        private bool _showHelp = true;

        public Form1()
        {
            InitializeComponent();

            Text = "Tactical Strike 3D — FPS Engine";
            ClientSize = new Size(1100, 680);
            StartPosition = FormStartPosition.CenterScreen;
            DoubleBuffered = true;
            KeyPreview = true;
            BackColor = Color.Black;

            SoundManager.Initialize();
            TextureManager.Initialize();

            _player = new Player();
            _renderer = new RaycastRenderer(640, 360);

            _weapons = new Weapon[]
            {
                new Weapon(WeaponType.Ak47),
                new Weapon(WeaponType.Deagle),
                new Weapon(WeaponType.Knife)
            };

            _enemies = new List<Enemy>
            {
                new Enemy(8.5, 3.5),
                new Enemy(18.5, 3.5),
                new Enemy(11.5, 9.5),
                new Enemy(5.5, 14.5),
                new Enemy(11.5, 17.5),
                new Enemy(20.5, 19.5)
            };

            _stopwatch = Stopwatch.StartNew();
            _lastTicks = _stopwatch.ElapsedTicks;

            _gameLoopTimer = new System.Windows.Forms.Timer();
            _gameLoopTimer.Interval = 15; // ~60 FPS
            _gameLoopTimer.Tick += GameLoop;
            _gameLoopTimer.Start();
        }

        private Weapon ActiveWeapon => _weapons[_currentWeaponIndex];

        private void GameLoop(object? sender, EventArgs e)
        {
            long currentTicks = _stopwatch.ElapsedTicks;
            float dt = (float)(currentTicks - _lastTicks) / Stopwatch.Frequency;
            _lastTicks = currentTicks;

            if (dt > 0.1f) dt = 0.1f;

            UpdateGame(dt);
            Invalidate();
        }

        private void UpdateGame(float dt)
        {
            // Round timer countdown
            _roundTimeRemaining -= dt;
            if (_roundTimeRemaining <= 0)
            {
                _roundTimeRemaining = 115f; // Reset round
            }

            // Keyboard look
            if (_rotateLeftKey) _player.Rotate(-2.5 * dt);
            if (_rotateRightKey) _player.Rotate(2.5 * dt);
            if (_lookUpKey) _player.LookVertical(120f * dt);
            if (_lookDownKey) _player.LookVertical(-120f * dt);

            // Player physics & movement
            _player.Update(dt);

            // Doors physics & proximity
            GameMap.UpdateDoors(dt);
            CheckDoorProximity();

            // Active weapon
            ActiveWeapon.Update(dt);

            if (_isLeftMouseDown)
            {
                if (ActiveWeapon.Type == WeaponType.Ak47)
                {
                    TryShoot();
                }
            }

            // Enemies update
            for (int i = 0; i < _enemies.Count; i++)
            {
                _enemies[i].Update(dt, _player.X, _player.Y);
            }

            // Timers
            if (_hitMarkerTimer > 0)
            {
                _hitMarkerTimer -= dt;
                if (_hitMarkerTimer < 0) _hitMarkerTimer = 0;
            }

            // Killfeed fade
            for (int i = _killfeedTimers.Count - 1; i >= 0; i--)
            {
                _killfeedTimers[i] -= dt;
                if (_killfeedTimers[i] <= 0)
                {
                    _killfeed.RemoveAt(i);
                    _killfeedTimers.RemoveAt(i);
                }
            }
        }

        private void CheckDoorProximity()
        {
            _nearDoor = false;
            int px = (int)Math.Floor(_player.X);
            int py = (int)Math.Floor(_player.Y);

            for (int dy = -1; dy <= 1; dy++)
            {
                for (int dx = -1; dx <= 1; dx++)
                {
                    var door = GameMap.GetDoor(px + dx, py + dy);
                    if (door != null)
                    {
                        double dist = Math.Sqrt((door.X + 0.5 - _player.X) * (door.X + 0.5 - _player.X) + (door.Y + 0.5 - _player.Y) * (door.Y + 0.5 - _player.Y));
                        if (dist < 2.0)
                        {
                            _nearDoor = true;
                            if (dist < 1.35 && !door.IsOpening && door.OpenAmount < 0.5f)
                            {
                                door.Open();
                                SoundManager.PlayDoor();
                            }
                            return;
                        }
                    }
                }
            }
        }

        private void InteractDoor()
        {
            int frontX = (int)Math.Floor(_player.X + _player.DirX * 1.6);
            int frontY = (int)Math.Floor(_player.Y + _player.DirY * 1.6);
            var door = GameMap.GetDoor(frontX, frontY);

            if (door == null)
            {
                int px = (int)Math.Floor(_player.X);
                int py = (int)Math.Floor(_player.Y);
                for (int dy = -1; dy <= 1 && door == null; dy++)
                {
                    for (int dx = -1; dx <= 1 && door == null; dx++)
                    {
                        var d = GameMap.GetDoor(px + dx, py + dy);
                        if (d != null)
                        {
                            door = d;
                            break;
                        }
                    }
                }
            }

            if (door != null)
            {
                door.Toggle();
                SoundManager.PlayDoor();
            }
        }

        private void TryShoot()
        {
            if (ActiveWeapon.Fire())
            {
                PerformHitscan();
            }
        }

        private void PerformHitscan()
        {
            double rayX = _player.X;
            double rayY = _player.Y;
            double dirX = _player.DirX;
            double dirY = _player.DirY;

            if (_player.MoveForward || _player.MoveBackward || _player.StrafeLeft || _player.StrafeRight)
            {
                var rnd = new Random();
                double spreadAngle = (rnd.NextDouble() * 2.0 - 1.0) * (_player.IsCrouching ? 0.02 : 0.045);
                double oldDirX = dirX;
                dirX = dirX * Math.Cos(spreadAngle) - dirY * Math.Sin(spreadAngle);
                dirY = oldDirX * Math.Sin(spreadAngle) + dirY * Math.Cos(spreadAngle);
            }

            double maxDist = 30.0;
            for (double d = 0.2; d < maxDist; d += 0.1)
            {
                double cx = rayX + dirX * d;
                double cy = rayY + dirY * d;
                if (GameMap.IsWall(cx, cy))
                {
                    maxDist = d;
                    break;
                }
            }

            if (ActiveWeapon.Type == WeaponType.Knife && maxDist > 2.0)
            {
                maxDist = 2.0;
            }

            Enemy? hitEnemy = null;
            double closestEnemyDist = maxDist;
            int hitEnemyIndex = -1;

            for (int i = 0; i < _enemies.Count; i++)
            {
                var enemy = _enemies[i];
                if (enemy.IsDead) continue;

                double vx = enemy.X - rayX;
                double vy = enemy.Y - rayY;

                double dot = vx * dirX + vy * dirY;
                if (dot > 0.3 && dot < closestEnemyDist)
                {
                    double projX = rayX + dirX * dot;
                    double projY = rayY + dirY * dot;
                    double distToRay = Math.Sqrt((enemy.X - projX) * (enemy.X - projX) + (enemy.Y - projY) * (enemy.Y - projY));

                    if (distToRay < 0.45)
                    {
                        closestEnemyDist = dot;
                        hitEnemy = enemy;
                        hitEnemyIndex = i + 1;
                    }
                }
            }

            if (hitEnemy != null)
            {
                var rnd = new Random();
                bool headshot = rnd.NextDouble() < 0.35 || (ActiveWeapon.Type == WeaponType.Deagle && closestEnemyDist < 6.0);
                int damage = ActiveWeapon.Damage * (headshot ? ActiveWeapon.HeadshotMultiplier : 1);

                bool killed = hitEnemy.TakeDamage(damage);
                SoundManager.PlayHit(headshot);
                _hitMarkerTimer = 0.22f;

                if (killed)
                {
                    string hsIcon = headshot ? " (HEADSHOT! 🎯)" : "";
                    _killfeed.Add($"You [{ActiveWeapon.Name}] Enemy #{hitEnemyIndex}{hsIcon}");
                    _killfeedTimers.Add(4.5f);
                }
            }
        }

        protected override void OnPaint(PaintEventArgs e)
        {
            base.OnPaint(e);

            _renderer.RenderScene(
                _player,
                _enemies,
                ActiveWeapon,
                e.Graphics,
                ClientSize.Width,
                ClientSize.Height,
                _hitMarkerTimer,
                _killfeed,
                _roundTimeRemaining,
                _nearDoor);

            if (_showHelp)
            {
                DrawHelpOverlay(e.Graphics);
            }
        }

        private void DrawHelpOverlay(Graphics g)
        {
            using var bg = new SolidBrush(Color.FromArgb(180, 0, 0, 0));
            using var font = new Font("Segoe UI", 10, FontStyle.Regular);
            using var fontBold = new Font("Segoe UI", 11, FontStyle.Bold);
            using var brush = new SolidBrush(Color.White);
            using var titleBrush = new SolidBrush(Color.FromArgb(240, 185, 50));

            int x = 25, y = 145, w = 340, h = 210;
            g.FillRectangle(bg, x, y, w, h);
            g.DrawRectangle(Pens.Gray, x, y, w, h);

            g.DrawString("УПРАВЛЕНИЕ (3D FPS Demo):", fontBold, titleBrush, x + 10, y + 8);
            g.DrawString("• W, A, S, D — Ходьба и стрейф", font, brush, x + 10, y + 32);
            g.DrawString("• Пробел (Space) — ПРЫЖОК", font, titleBrush, x + 10, y + 54);
            g.DrawString("• Ctrl / C — ПРИСЯДКА (Duck)", font, titleBrush, x + 10, y + 76);
            g.DrawString("• E — ОТКРЫТЬ ДВЕРЬ", font, titleBrush, x + 10, y + 98);
            g.DrawString("• Мышь — Вверх/Вниз/Поворот + ЛКМ (Стрельба)", font, brush, x + 10, y + 120);
            g.DrawString("• 1, 2, 3 — Выбор оружия (AK-47, Deagle, Нож)", font, brush, x + 10, y + 142);
            g.DrawString("• R — Перезарядка", font, brush, x + 10, y + 164);
            g.DrawString("• H — Скрыть / Показать эту подсказку", font, titleBrush, x + 10, y + 186);
        }

        protected override void OnMouseDown(MouseEventArgs e)
        {
            base.OnMouseDown(e);

            if (e.Button == MouseButtons.Left)
            {
                if (!_isMouseCaptured)
                {
                    CaptureMouse();
                }
                else
                {
                    _isLeftMouseDown = true;
                    TryShoot();
                }
            }
            else if (e.Button == MouseButtons.Right)
            {
                SwitchWeapon((_currentWeaponIndex + 1) % _weapons.Length);
            }
        }

        protected override void OnMouseUp(MouseEventArgs e)
        {
            base.OnMouseUp(e);
            if (e.Button == MouseButtons.Left)
            {
                _isLeftMouseDown = false;
            }
        }

        protected override void OnMouseMove(MouseEventArgs e)
        {
            base.OnMouseMove(e);

            if (_isMouseCaptured)
            {
                int centerX = ClientSize.Width / 2;
                int centerY = ClientSize.Height / 2;

                int dx = e.X - centerX;
                int dy = e.Y - centerY;

                if (dx != 0 || dy != 0)
                {
                    _player.Rotate(dx * _mouseSensitivity);
                    // Moving mouse up decreases dy, so -dy looks up (positive pitch)
                    _player.LookVertical(-dy * 0.85f);
                    ResetCursorToCenter();
                }
            }
        }

        private void CaptureMouse()
        {
            _isMouseCaptured = true;
            Cursor.Hide();
            ResetCursorToCenter();
        }

        private void ReleaseMouse()
        {
            if (_isMouseCaptured)
            {
                _isMouseCaptured = false;
                Cursor.Show();
            }
        }

        private void ResetCursorToCenter()
        {
            Point center = PointToScreen(new Point(ClientSize.Width / 2, ClientSize.Height / 2));
            Cursor.Position = center;
        }

        protected override void OnKeyDown(KeyEventArgs e)
        {
            base.OnKeyDown(e);

            switch (e.KeyCode)
            {
                case Keys.W: _player.MoveForward = true; break;
                case Keys.S: _player.MoveBackward = true; break;
                case Keys.A: _player.StrafeLeft = true; break;
                case Keys.D: _player.StrafeRight = true; break;

                // Jump
                case Keys.Space:
                    _player.Jump();
                    break;

                // Crouch
                case Keys.ControlKey:
                case Keys.C:
                    _player.IsCrouching = true;
                    break;

                // Open Door
                case Keys.E:
                    InteractDoor();
                    break;

                // Shift Walk
                case Keys.ShiftKey:
                    _player.IsRunning = false;
                    break;

                // Arrow / keyboard look
                case Keys.Q:
                case Keys.Left:
                    _rotateLeftKey = true;
                    break;
                case Keys.Right:
                    _rotateRightKey = true;
                    break;
                case Keys.Up:
                case Keys.PageUp:
                    _lookUpKey = true;
                    break;
                case Keys.Down:
                case Keys.PageDown:
                    _lookDownKey = true;
                    break;

                case Keys.D1: SwitchWeapon(0); break;
                case Keys.D2: SwitchWeapon(1); break;
                case Keys.D3: SwitchWeapon(2); break;

                case Keys.R: ActiveWeapon.Reload(); break;
                case Keys.H: _showHelp = !_showHelp; break;

                case Keys.Escape:
                    ReleaseMouse();
                    break;
            }
        }

        protected override void OnKeyUp(KeyEventArgs e)
        {
            base.OnKeyUp(e);

            switch (e.KeyCode)
            {
                case Keys.W: _player.MoveForward = false; break;
                case Keys.S: _player.MoveBackward = false; break;
                case Keys.A: _player.StrafeLeft = false; break;
                case Keys.D: _player.StrafeRight = false; break;

                case Keys.ControlKey:
                case Keys.C:
                    _player.IsCrouching = false;
                    break;

                case Keys.ShiftKey:
                    _player.IsRunning = true;
                    break;

                case Keys.Q:
                case Keys.Left:
                    _rotateLeftKey = false;
                    break;
                case Keys.Right:
                    _rotateRightKey = false;
                    break;
                case Keys.Up:
                case Keys.PageUp:
                    _lookUpKey = false;
                    break;
                case Keys.Down:
                case Keys.PageDown:
                    _lookDownKey = false;
                    break;
            }
        }

        private void SwitchWeapon(int index)
        {
            if (index >= 0 && index < _weapons.Length && index != _currentWeaponIndex)
            {
                _currentWeaponIndex = index;
                ActiveWeapon.TimeSinceLastShot = ActiveWeapon.FireRate;
            }
        }

        protected override void OnMouseWheel(MouseEventArgs e)
        {
            base.OnMouseWheel(e);
            if (e.Delta > 0)
            {
                int next = (_currentWeaponIndex + 1) % _weapons.Length;
                SwitchWeapon(next);
            }
            else if (e.Delta < 0)
            {
                int prev = (_currentWeaponIndex - 1 + _weapons.Length) % _weapons.Length;
                SwitchWeapon(prev);
            }
        }

        protected override void OnDeactivate(EventArgs e)
        {
            base.OnDeactivate(e);
            ReleaseMouse();
        }
    }
}
