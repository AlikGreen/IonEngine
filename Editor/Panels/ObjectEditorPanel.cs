using System.Numerics;
using System.Reflection;
using System.Runtime.InteropServices;
using ImGuiNET;
using Newtonsoft.Json;

namespace IonEditor;

public class ObjectEditorPanel
{
    public object obj;
    public string filePath;

    public void Draw()
    {
        ImGui.Begin("Object Editor");
        if(obj != null)
        {
            float dragSpeed = 0.05f;

            if(ImGui.GetIO().KeyShift)
            {
                dragSpeed = 0.3f;
            }
            else if(ImGui.GetIO().KeyCtrl)
            {
                dragSpeed = 0.002f;
            }
            DisplayFields(obj, dragSpeed);

            ImGui.Dummy(new Vector2(0, ImGui.GetContentRegionAvail().Y - ImGui.GetFrameHeightWithSpacing()));

            if (ImGui.Button("Save"))
            {
                Save(obj);
            }
        }
        ImGui.End();
    }

    public void Save(object obj)
    {
        if(obj != null)
        {
            try
            {
                JsonSerializerSettings settings = new JsonSerializerSettings
                {
                    PreserveReferencesHandling = PreserveReferencesHandling.All,
                    TypeNameHandling = TypeNameHandling.All
                };
                string fileText = JsonConvert.SerializeObject(obj, settings);
                File.WriteAllText(filePath, fileText);
            }
            catch(Exception e)
            {
                Console.WriteLine(e);
            }
        }
    }

    public string FormatFieldName(string fieldName)
    {
        // Replace underscores with spaces
        fieldName = fieldName.Replace("_", " ");

        // Insert a space before each uppercase letter that is either preceded by a lowercase letter or followed by a lowercase letter (i.e., it's not an acronym)
        fieldName = System.Text.RegularExpressions.Regex.Replace(fieldName, "(?<=[a-z])([A-Z])|([A-Z])(?=[a-z])", " $1$2");

        // Capitalize the first letter of each word
        fieldName = System.Globalization.CultureInfo.CurrentCulture.TextInfo.ToTitleCase(fieldName.ToLower());

        return fieldName;
    }

    private unsafe void DisplayFields(object obj, float dragSpeed)
    {
        FieldInfo[] fields = obj.GetType().GetFields();

        for (int j = 0; j < fields.Length; j++)
        {
            bool showInInspector = fields[j].GetCustomAttribute(typeof(ShowInInspector)) != null;
            bool hideInInspector = fields[j].GetCustomAttribute(typeof(HideInInspector)) != null;
            AcceptsFiles acceptsFilesAttribute = fields[j].GetCustomAttribute(typeof(AcceptsFiles)) as AcceptsFiles;

            object fieldValue = fields[j].GetValue(obj);
            string fieldName = FormatFieldName(fields[j].Name);

            if ((!showInInspector && fields[j].IsPrivate) || hideInInspector || fields[j].Name == "gameObject")
            {
                continue;
            }

            if(fieldValue is string && acceptsFilesAttribute != null)
            {
                ImGui.Text(fieldName);


                ImGui.SameLine();
                ImGui.PushStyleColor(ImGuiCol.Button, new Vector4(0.5f, 0.5f, 0.5f, 0.2f));
                ImGui.PushStyleColor(ImGuiCol.ButtonHovered, new Vector4(0.5f, 0.5f, 0.5f, 0.4f));
                ImGui.PushStyleColor(ImGuiCol.ButtonActive, new Vector4(0.5f, 0.5f, 0.5f, 0.2f));
                ImGui.PushStyleVar(ImGuiStyleVar.FrameRounding, 0);
                ImGui.PushStyleVar(ImGuiStyleVar.FramePadding, new Vector2(0));
                string value = (string)fieldValue;
                ImGui.Button(Path.GetFileName(value), new Vector2(ImGui.GetContentRegionAvail().X, 13));
                ImGui.PopStyleColor(3);
                ImGui.PopStyleVar(2);

                if(ImGui.BeginDragDropTarget())
                {
                    ImGuiPayloadPtr payload = ImGui.AcceptDragDropPayload("CONTENT_BROWSER_ITEM");
                    if(payload.NativePtr != null)
                    {
                        string droppedFile = Marshal.PtrToStringUTF8((IntPtr)payload.Data.ToPointer(), payload.DataSize);
                        string extension = Path.GetExtension(droppedFile);

                        if(acceptsFilesAttribute != null && acceptsFilesAttribute.Extensions.Contains(extension))
                        {
                            fields[j].SetValue(obj, droppedFile);
                        }
                    }
                    ImGui.EndDragDropTarget();
                }
            }
            else if (fieldValue is string)
            {
                string value = (string)fieldValue;
                if (ImGui.InputText(fieldName, ref value, 100))
                {
                    fields[j].SetValue(obj, value);
                }
            }
            else if (fieldValue is float)
            {
                float value = (float)fieldValue;
                if (ImGui.DragFloat(fieldName, ref value, dragSpeed))
                {
                    fields[j].SetValue(obj, value);
                }
            }
            else if (fieldValue is Vector2)
            {
                Vector2 value = (Vector2)fieldValue;
                if (ImGui.DragFloat2(fieldName, ref value, dragSpeed))
                {
                    fields[j].SetValue(obj, value);
                }
            }
            else if (fieldValue is Vector3)
            {
                Vector3 value = (Vector3)fieldValue;
                if (ImGui.DragFloat3(fieldName, ref value, dragSpeed))
                {
                    fields[j].SetValue(obj, value);
                }
            }
            else if (fieldValue is Vector4)
            {
                Vector4 value = (Vector4)fieldValue;
                if (ImGui.DragFloat4(fieldName, ref value, dragSpeed))
                {
                    fields[j].SetValue(obj, value);
                }
            }
            else if (fieldValue is bool)
            {
                bool value = (bool)fieldValue;
                if (ImGui.Checkbox(fieldName, ref value))
                {
                    fields[j].SetValue(obj, value);
                }
            }
            else if (fieldValue is Enum)
            {
                string[] names = Enum.GetNames(fieldValue.GetType());
                int current = Array.IndexOf(names, fieldValue.ToString());
                if (ImGui.Combo(fieldName, ref current, names, names.Length))
                {
                    object newValue = Enum.Parse(fieldValue.GetType(), names[current]);
                    fields[j].SetValue(obj, newValue);
                }
            }
            else
            {
                ImGui.Text(fieldName);
            }
        }
    }
}