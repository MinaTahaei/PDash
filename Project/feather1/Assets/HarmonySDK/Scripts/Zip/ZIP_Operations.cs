
#if UNITY_ANDROID

using UnityEngine;

using System.Collections;
using System.Collections.Generic;
using System.IO;

using Pathfinding.Ionic.Zip;

/*!
 *  @class ZIP_Operations
 *  Operations on zip files.
 */
public class ZIP_Operations
{
  public static void unzipFolder( string zipFilename, string folderPath, string dstFolder )
  {
    byte[] data = new byte[4096];

    using (ZipInputStream inputStream = new ZipInputStream(File.OpenRead(zipFilename)))
    {
      ZipEntry entry;
      while ((entry = inputStream.GetNextEntry()) != null)
      {
        if ( entry.FileName.StartsWith( folderPath ) && !entry.IsDirectory )
        {
          char[] charsToTrim = { '/' };
          string entryName = entry.FileName.Substring( folderPath.Length ).Trim( charsToTrim );
          string entryFilename = entryName;
          string entryFolder = dstFolder;

          int idx = entryFilename.LastIndexOf( "/" );
          if ( idx >= 0 )
          {
            entryFolder = dstFolder + "/" + entryFilename.Substring( 0, idx );
            entryFilename = entryFilename.Substring( idx+1 );
          }

          DirectoryInfo dirInfo = new DirectoryInfo(entryFolder);
          if ( !dirInfo.Exists )
          {
            Directory.CreateDirectory( entryFolder );
          }

          Message.Log( "Copying file to '" + entryFolder + "/" + entryFilename + "'" );
          FileStream outputStream = new FileStream( entryFolder + "/" + entryFilename, FileMode.Create, FileAccess.Write );

          int size = inputStream.Read( data, 0, data.Length );
          while ( size > 0 )
          {
            outputStream.Write( data, 0, size );
            size = inputStream.Read( data, 0, data.Length );
          }

          outputStream.Close();
        }
      }
    }
  }

  public static string[] getFiles( string zipFilename, string folderPath )
  {
    List<string> fileList = new List<string>();

    using (ZipInputStream inputStream = new ZipInputStream(File.OpenRead(zipFilename)))
    {
      ZipEntry entry;
      while ((entry = inputStream.GetNextEntry()) != null)
      {
        if ( entry.FileName.StartsWith( folderPath ) && !entry.IsDirectory )
        {
          char[] charsToTrim = { '/' };
          string entryFilename = entry.FileName.Substring( folderPath.Length ).Trim( charsToTrim );
          //Message.Log("entryFilename: '" + entryFilename + "'" );

          //  Must not be a nested file in a subdirectory.
          int idx = entryFilename.LastIndexOf( "/" );
          if ( idx < 0 )
          {
            fileList.Add(entryFilename);
          }
        }
      }
    }

    return fileList.ToArray();
  }

  public static string[] getDirectories( string zipFilename, string folderPath )
  {
    List<string> dirList = new List<string>();

    using (ZipInputStream inputStream = new ZipInputStream(File.OpenRead(zipFilename)))
    {
      ZipEntry entry;
      while ((entry = inputStream.GetNextEntry()) != null)
      {
        if ( entry.FileName.StartsWith( folderPath )  )
        {
          char[] charsToTrim = { '/' };
          string entryName = entry.FileName.Substring( folderPath.Length ).Trim( charsToTrim );
          //Message.Log("entryName: '" + entryName + "'" );

          // extract first directory.
          int idx = entryName.IndexOf( "/" );
          if ( idx >= 0 )
          {
            string dirEntry = entryName.Substring( 0, idx );
            if ( !dirList.Contains( dirEntry ) )
              dirList.Add( entryName.Substring( 0, idx ) );
          }
        }
      }
    }

    return dirList.ToArray();
  }
}

#endif

