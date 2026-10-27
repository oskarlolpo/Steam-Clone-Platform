using System;
using System.Collections.Generic;

namespace CS2WinFormsDemo
{
    public static class GameMap
    {
        public const int Width = 24;
        public const int Height = 24;

        // 0: Empty, 1: Sandstone, 2: Wood Crate, 3: Metal Sliding Door, 4: Site A Concrete
        public static readonly int[,] Grid = new int[Height, Width]
        {
            {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
            {1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1},
            {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,2,2,0,0,0,1},
            {1,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,1,0,2,2,0,0,0,1},
            {1,0,0,0,0,0,1,0,0,1,1,3,1,1,0,0,1,0,0,0,0,4,4,1},
            {1,1,3,1,1,1,1,0,0,1,0,0,0,1,0,0,1,1,1,0,0,4,4,1},
            {1,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1},
            {1,0,2,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1},
            {1,0,2,2,0,0,1,1,1,1,0,0,0,1,1,1,1,0,1,1,3,1,1,1},
            {1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1},
            {1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1},
            {1,1,1,3,1,1,1,0,0,2,2,0,2,2,0,0,1,1,1,3,1,1,0,1},
            {1,0,0,0,0,0,0,0,0,2,2,0,2,2,0,0,0,0,0,0,0,1,0,1},
            {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1},
            {1,0,2,2,0,0,1,1,0,0,0,0,0,0,0,1,1,0,0,2,2,1,0,1},
            {1,0,2,0,0,0,1,1,0,0,0,0,0,0,0,1,1,0,0,0,0,1,0,1},
            {1,0,0,0,0,0,0,1,0,0,1,1,1,0,0,1,0,0,0,0,0,1,0,1},
            {1,0,0,0,0,0,0,1,0,0,1,4,1,0,0,1,0,0,0,0,0,1,0,1},
            {1,1,1,1,0,0,0,1,1,3,1,4,1,3,1,1,0,0,0,1,1,1,0,1},
            {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
            {1,0,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,0,1},
            {1,0,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,0,1},
            {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
            {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
        };

        public static Dictionary<long, Door> Doors = new Dictionary<long, Door>();

        static GameMap()
        {
            InitializeDoors();
        }

        public static void InitializeDoors()
        {
            Doors.Clear();
            for (int y = 0; y < Height; y++)
            {
                for (int x = 0; x < Width; x++)
                {
                    if (Grid[y, x] == 3)
                    {
                        long key = ((long)x << 32) | (uint)y;
                        Doors[key] = new Door(x, y);
                    }
                }
            }
        }

        public static Door? GetDoor(int x, int y)
        {
            long key = ((long)x << 32) | (uint)y;
            if (Doors.TryGetValue(key, out var door))
                return door;
            return null;
        }

        public static void UpdateDoors(float dt)
        {
            foreach (var door in Doors.Values)
            {
                door.Update(dt);
            }
        }

        public static bool IsWall(double x, double y)
        {
            int ix = (int)Math.Floor(x);
            int iy = (int)Math.Floor(y);
            if (ix < 0 || ix >= Width || iy < 0 || iy >= Height)
                return true;

            int tile = Grid[iy, ix];
            if (tile == 0) return false;

            if (tile == 3)
            {
                var door = GetDoor(ix, iy);
                if (door != null && door.OpenAmount >= 0.7f)
                {
                    return false; // Walkable when open!
                }
                return true;
            }

            return true;
        }

        public static int GetWall(int x, int y)
        {
            if (x < 0 || x >= Width || y < 0 || y >= Height)
                return 1;
            return Grid[y, x];
        }
    }
}
