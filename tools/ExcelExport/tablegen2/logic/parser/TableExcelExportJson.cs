using System.Collections.Generic;
using System.Linq;
using System.Text;
using Newtonsoft.Json;
using System.IO;

namespace tablegen2.logic
{
    public static class TableExcelExportJson
    {
        public static void exportExcelFile(TableExcelData data, string filePath)
        {
            Dictionary<object, Dictionary<string, object>> record =
                new Dictionary<object, Dictionary<string, object>>();
            List<Dictionary<string, object>> lst = data.Rows.Select(a =>
            {
                var r = new Dictionary<string, object>();
                for (int i = 0; i < data.Headers.Count; i++)
                {
                    var hdr = data.Headers[i];
                    var val = a.StrList[i];
                    object obj = null;
                    switch (hdr.FieldType)
                    {
                        case "string":
                        case "formula":
                            obj = val;
                            break;
                        case "int":
                            {
                                int n = 0;
                                int.TryParse(val, out n);
                                obj = n;
                            }
                            break;
                        case "double":
                            {
                                double n = 0;
                                double.TryParse(val, out n);
                                obj = n;
                            }
                            break;
                    }
                    r[hdr.FieldName] = obj;
                    if (hdr.FieldName == "Id")
                    {
                        record[obj] = r;
                    }
                }
                return r;
            }).ToList();

            Dictionary<string, Dictionary<object, Dictionary<string, object>>> jsondata =
                new Dictionary<string, Dictionary<object, Dictionary<string, object>>>();
            jsondata["data"] = record;

            string output = JsonConvert.SerializeObject(jsondata, Formatting.Indented);
            File.WriteAllBytes(filePath, Encoding.UTF8.GetBytes(output));
        }
    }
}
