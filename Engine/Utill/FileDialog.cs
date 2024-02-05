using System;
using System.Runtime.InteropServices;
using System.Text;

public class FileSystemDialog
{
    [DllImport("comdlg32.dll", CharSet = CharSet.Auto, SetLastError = true)]
    private static extern bool GetOpenFileName(ref OPENFILENAME lpofn);

    
    [DllImport("shell32.dll")]
    private static extern IntPtr SHBrowseForFolder(ref BROWSEINFO lpbi);

    [DllImport("shell32.dll", CharSet = CharSet.Auto)]
    private static extern bool SHGetPathFromIDList(IntPtr pidl, StringBuilder pszPath);

    [StructLayout(LayoutKind.Sequential)]
    private struct BROWSEINFO
    {
        public IntPtr hwndOwner;
        public IntPtr pidlRoot;
        public IntPtr pszDisplayName;
        public string lpszTitle;
        public uint ulFlags;
        public IntPtr lpfn;
        public int lParam;
        public IntPtr iImage;
    }

    public string FolderName { get; private set; }

    public bool ShowFolderDialog(string startFolder = "", string dialogTitle = "Select a folder")
    {
        StringBuilder path = new StringBuilder(256);
        BROWSEINFO bi = new BROWSEINFO
        {
            lpszTitle = dialogTitle,
            ulFlags = 0x0001 // BIF_RETURNONLYFSDIRS
        };

        if (!string.IsNullOrEmpty(startFolder))
        {
            bi.pidlRoot = Marshal.StringToCoTaskMemUni(startFolder);
        }
    
        IntPtr pidl = SHBrowseForFolder(ref bi);
        if (pidl != IntPtr.Zero)
        {
            if (SHGetPathFromIDList(pidl, path))
            {
                FolderName = path.ToString();
                return true;
            }
        }
    
        return false;
    }

    [DllImport("comdlg32.dll", CharSet = CharSet.Auto, SetLastError = true)]
    private static extern bool GetSaveFileName(ref OPENFILENAME lpofn);

    public bool ShowSaveFileDialog(string initialDirectory = "", params string[] fileTypes)
    {
        OPENFILENAME ofn = new OPENFILENAME();
        ofn.lStructSize = Marshal.SizeOf(ofn);
        ofn.lpstrFile = new string(new char[256]);
        ofn.nMaxFile = ofn.lpstrFile.Length;
        ofn.lpstrFilter = string.Join("\0", fileTypes) + "\0";
        ofn.nFilterIndex = 1;
        ofn.lpstrFileTitle = new string(new char[64]);
        ofn.nMaxFileTitle = ofn.lpstrFileTitle.Length;
        if(initialDirectory == "")
            ofn.lpstrInitialDir = Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().Location);
        else
            ofn.lpstrInitialDir = initialDirectory;
        ofn.Flags = 0;

        bool result = GetSaveFileName(ref ofn);
        if (result)
        {
            FileName = ofn.lpstrFile;
        }

        return result;
    }


    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Auto)]
    private struct OPENFILENAME
    {
        public int lStructSize;
        public IntPtr hwndOwner;
        public IntPtr hInstance;
        public string lpstrFilter;
        public string lpstrCustomFilter;
        public int nMaxCustFilter;
        public int nFilterIndex;
        public string lpstrFile;
        public int nMaxFile;
        public string lpstrFileTitle;
        public int nMaxFileTitle;
        public string lpstrInitialDir;
        public string lpstrTitle;
        public int Flags;
        public short nFileOffset;
        public short nFileExtension;
        public string lpstrDefExt;
        public IntPtr lCustData;
        public IntPtr lpfnHook;
        public string lpTemplateName;
        public IntPtr pvReserved;
        public int dwReserved;
        public int FlagsEx;
    }

    public string FileName { get; private set; }

    public bool ShowFileDialog(string initialDirectory = "", params string[] fileTypes)
    {
        OPENFILENAME ofn = new OPENFILENAME();
        ofn.lStructSize = Marshal.SizeOf(ofn);
        ofn.lpstrFile = new string(new char[256]);
        ofn.nMaxFile = ofn.lpstrFile.Length;
        ofn.lpstrFilter = string.Join("\0", fileTypes) + "\0";
        ofn.nFilterIndex = 1;
        ofn.lpstrFileTitle = new string(new char[64]);
        ofn.nMaxFileTitle = ofn.lpstrFileTitle.Length;
        if(initialDirectory == "")
            ofn.lpstrInitialDir = Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().Location);
        else
            ofn.lpstrInitialDir = initialDirectory;
        ofn.Flags = 0;

        bool result = GetOpenFileName(ref ofn);
        if (result)
        {
            FileName = ofn.lpstrFile;
        }

        return result;
    }
}
