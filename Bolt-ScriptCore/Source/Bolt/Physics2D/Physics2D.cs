namespace Bolt
{
    public struct RaycastHit2D
    {
        public Entity? Entity;
        public Vector2 Point;
        public Vector2 Normal;
        public bool Hit;
    }

    public static class Physics2D
    {

        public static RaycastHit2D Raycast(Vector2 origin, Vector2 direction, float maxDistance = Mathf.Infinity)
        {
            RaycastHit2D result = new();

            bool hit = InternalCalls.Physics2D_Raycast(
                origin.X, origin.Y,
                direction.X, direction.Y,
                maxDistance,
                out ulong hitEntityID,
                out float hitX, out float hitY,
                out float hitNormalX, out float hitNormalY
            );

            result.Hit = hit;
            if (hit)
            {
                result.Entity = new Entity(hitEntityID);
                result.Point = new Vector2(hitX, hitY);
                result.Normal = new Vector2(hitNormalX, hitNormalY);
            }

            return result;
        }
    }
}
