namespace Bolt
{
    /// <summary>
    /// Result of a 2D raycast query.
    /// </summary>
    public struct RaycastHit2D
    {
        /// <summary>The entity that was hit, or null if nothing was hit.</summary>
        public Entity? Entity;

        /// <summary>World-space point where the ray hit.</summary>
        public Vector2 Point;

        /// <summary>Surface normal at the hit point.</summary>
        public Vector2 Normal;

        /// <summary>Whether the raycast hit anything.</summary>
        public bool Hit;
    }

    /// <summary>
    /// 2D physics query interface. Wraps Bolt-Engine's Box2D-based physics.
    /// </summary>
    public static class Physics2D
    {
        /// <summary>
        /// Cast a ray from origin in the given direction up to maxDistance.
        /// Returns a RaycastHit2D with the result.
        /// </summary>
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
