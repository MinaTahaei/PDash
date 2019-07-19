using UnityEngine;

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;
using System.Threading;

namespace WebGL_FS
{
#if UNITY_WEBGL && !UNITY_EDITOR
    public class ExternalWebGL_FS
    {
        [DllImport ("__Internal")]
        public static extern int GetNumberOfFoldersUnderFolder( string folderName );

        [DllImport("__Internal")]
        public static extern IntPtr FolderUnderFolderForNumber(string folderName, int number);

        [DllImport ("__Internal")]
        public static extern bool ResourceAccessable( string projectFolder );
    
        [DllImport ("__Internal")]
        public static extern void PrepareResource( string resourceName );

        [DllImport ("__Internal")]
        public static extern void RemovePreparedResource(string resourceName);

        //0: resource not yet queued/prepared/failed;
        //1: resource Failed Preparation! not ready to use
        //2: resource already queued for preparing
        //3: resource already prepared, ready to use
        [DllImport("__Internal")]
        public static extern int GetStateOfResource(string resourceName);
        
    }

    public class WebGL_FileSystem
    {
        public static string[] GetDirectories(string directoryName)
        {
            List<string> dirList = new List<string>();

            int numberOfFolders = ExternalWebGL_FS.GetNumberOfFoldersUnderFolder(directoryName);

            for (int i = 0; i < numberOfFolders; i++)
            {
                string folderName = Marshal.PtrToStringAnsi(ExternalWebGL_FS.FolderUnderFolderForNumber(directoryName, i));
                dirList.Add(folderName);
            }

            return dirList.ToArray();
        }

        public static bool ResourceReadyCheck(string resourceName)
        {
            return (ExternalWebGL_FS.GetStateOfResource(resourceName)==3);
        }

        public static void PrepareProjectFolderResource(string resourceName)
        {
            ExternalWebGL_FS.PrepareResource(resourceName);
            
        }

        public static void RemovePreparedResource(string resourceName){
            ExternalWebGL_FS.RemovePreparedResource(resourceName);
        }

        //0: resource not yet queued/prepared;
        //1: resource Failed Preparation! not ready to use
        //2: resource already queued for preparing
        //3: resource already prepared
        public static int GetPreparationStateOfResource(string resourceName)
        {
            return ExternalWebGL_FS.GetStateOfResource(resourceName);
        }
    }
#endif
}