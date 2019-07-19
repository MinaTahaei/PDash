
#if UNITY_ANDROID

using UnityEngine;

using System.Collections;
using System.Collections.Generic;
using System.IO;

/*!
 *  @class ZIP_AndroidAssets
 *  Android specific zip file operations.
 */
class ZIP_AndroidAssets
{
  private static string jarFileDescriptor = "jar:file://";
  private static string jarSeparator      = "!/";

  public static bool isUrl( string url )
  {
    return url.StartsWith( jarFileDescriptor );
  }

  public static bool extractPaths( string url, out string zipFilename, out string internalPath )
  {
    zipFilename = internalPath = null;  // empty string.

    if ( isUrl(url) )
    {
      //  Retrieve filename from url.
      zipFilename = url.Substring( jarFileDescriptor.Length );

      //  Second part of filename is internal folder in zip file.
      int idx = zipFilename.IndexOf( jarSeparator );
      if ( idx >= 0 )
      {
        string workingString = zipFilename;

        //  Retrieve filename from url.
        zipFilename = workingString.Substring( 0, idx );
        //  Retrieve internal path from url.
        internalPath = workingString.Substring( idx + jarSeparator.Length );
      }

      return true;
    }

    return false;
  }

  public static string zipFilename( string url )
  {
    string zipFilename, internalPath;
    extractPaths( url, out zipFilename, out internalPath );
    return zipFilename; // might be null.
  }


  public static string internalPath( string url )
  {
    string zipFilename, internalPath;
    extractPaths( url, out zipFilename, out internalPath );
    return internalPath; // might be null.
  }

  public static string[] retrieveProjectFolders( string url )
  {
    string zipFilename, internalPath;
    if ( extractPaths( url, out zipFilename, out internalPath ) )
    {
      if ( new FileInfo(zipFilename).Exists )
      {
        string[] dirList = ZIP_Operations.getDirectories(zipFilename, internalPath);
        return dirList;
      }
    }

    return new string[0]; //  empty array.
  }

  public static bool synchronizeProjectFolder( string url, out string projectFolder )
  {
    projectFolder = null;

    string zipFilename, internalPath;
    if ( extractPaths( url, out zipFilename, out internalPath ) )
    {
      string folderName = (new DirectoryInfo(internalPath)).Name;

      string tmpProjectFolder = Application.temporaryCachePath + "/" + folderName;

      FileInfo zipFileInfo = new FileInfo(zipFilename);
      if ( zipFileInfo.Exists )
      {
        DirectoryInfo tmpDirInfo = new DirectoryInfo(tmpProjectFolder);

        //Message.Log( "zipFileInfo: " + zipFileInfo.LastWriteTime.ToString() );
        //Message.Log( "tmpDirInfo: " + tmpDirInfo.LastWriteTime.ToString() );

        if ( tmpDirInfo.Exists && (tmpDirInfo.LastWriteTime > zipFileInfo.LastWriteTime) )
        {
          projectFolder = tmpProjectFolder;
          return true;
        }
        else
        {
          //  Copy new version of asset.
          if ( tmpDirInfo.Exists )
          {
            tmpDirInfo.Delete(true /*recursive*/);
          }

          //  Unzip project folder to temporary cache path.
          ZIP_Operations.unzipFolder( zipFilename, internalPath, tmpProjectFolder );

          if ( new DirectoryInfo(tmpProjectFolder).Exists )
          {
            projectFolder = tmpProjectFolder;
            return true;
          }
        }
      }
    }

    return false;
  }
}

#endif

