using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Bolt
{
    //INFO(Ben-Scr): Used for writing text above a field within the editor inspector ui
    public class HeaderAttribute : Attribute
    {
        public string Content;
    }
}
