
using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System;

/*!
 * @class HarmonyMeta
 * Harmony meta container data structure
 */
[Serializable]
public class HarmonyMeta
{

  /*!
   * @class MetaEntry
   * Harmony meta data structure
   */
  [Serializable]
  public class MetaEntry
  {
    public string metaName;
    public string nodeName;
    public string metaValue;
  }

  public List<MetaEntry> metas = new List<MetaEntry>();

  public int Count
  {
    get { return metas.Count; }
  }

  public void AddMeta(MetaEntry newEntry)
  {
    metas.Add(newEntry);
  }

  public void RemoveMeta(int index)
  {
    metas.RemoveAt(index);
  }

  public void ClearMeta()
  {
    metas.Clear();
  }

  public string GetMetaValue(string metaName)
  {
    if (metas.Count > 0)
    {
      MetaEntry query = metas.Find(entry => CompareString(entry.metaName, metaName));
      if (query != null)
        return query.metaValue;
    }

    return null;
  }

  public string GetMetaValue(string metaName, string nodeName)
  {
    if (metas.Count > 0)
    {
      MetaEntry query = metas.Find(entry => (CompareString(entry.metaName, metaName) && CompareString(entry.nodeName, nodeName)));
      if (query != null)
        return query.metaValue;
    }

    return null;
  }

  public MetaEntry GetMeta(string metaName)
  {
    if (metas.Count > 0)
    {
      MetaEntry query = metas.Find(entry => entry.metaName == metaName);
      return query;
    }

    return null;
  }

  public MetaEntry GetMeta(string metaName, string nodeName)
  {
    if (metas.Count > 0)
    {
      MetaEntry query = metas.Find(entry => (CompareString(entry.metaName, metaName) && CompareString(entry.nodeName, nodeName)));
      return query;
    }

    return null;
  }

  public MetaEntry[] GetMetas(string metaName)
  {
    if (metas.Count > 0)
    {
      IEnumerable<MetaEntry> query = metas.Where(entry => CompareString(entry.metaName, metaName));
      return query.ToArray();
    }

    return new MetaEntry[0];
  }

  public MetaEntry[] GetNodeMetas(string nodeName)
  {
    if (metas.Count > 0)
    {
      IEnumerable<MetaEntry> query = metas.Where(entry => CompareString(entry.nodeName, nodeName));
      return query.ToArray();
    }

    return new MetaEntry[0];
  }

  private bool CompareString( string string1, string string2 )
  {
    if ( string.IsNullOrEmpty(string1) &&
         string.IsNullOrEmpty(string2) )
    {
      return true;
    }

    return string1.Equals(string2);
  }
}

