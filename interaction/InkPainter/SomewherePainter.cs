using UnityEngine;

namespace Es.InkPainter
{
    public class SomewherePainter : MonoBehaviour
    {
        [SerializeField]
        private Brush brush = null; 
        

        public void PointPaint(Vector3 pos)
        {
            var canvas = GetComponent<InkCanvas>();
            if (canvas != null)
            {
                canvas.Paint(brush, pos);
            }
        }

        public void ScopePaint(Vector3 from, Vector3 to)
        {
            var canvas = GetComponent<InkCanvas>();
            if (canvas != null)
            {
                for (Vector3 pos = from; pos.x <= to.x; pos.x++)
                {
                    for (pos.z = from.z; pos.z <= to.z; pos.z++)
                        canvas.Paint(brush, pos);
                }
            }
        }
    }
}