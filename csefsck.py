# Below is a file system checker for our file system.
# As with any filesystem, there exists the possibility that #errors will be introduced.  In Linux, these errors are resolved #using a File System ChecKer (fsck).  Each fsck is custom #designed for the file system type so that it can examine #everything to make sure it is consistent.  


#!/usr/bin/env python

import os
from os import listdir
import json
import re
import time

BLOCKSIZE = 4096 
filesystemdatastructure = {}
filesystemdatastructure['fblocks'] = {}
filesystemdatastructure['intab'] = {}
MAXFREEPOINTERBLOCKS = 25
DATAPATH = 'data/'
DIRPFIX = 'd:'
FLPFIX = 'f:'


def restore_FS():

  for i in range(0,26):
    filename = 'fusedata.'+str(i)
    metadatafo = open(DATAPATH+filename,'r')
    metadatastring = metadatafo.read()
    metadatafo.close()

    jsondata = re.sub(r"(['\"])?([a-zA-Z0-9_.:]+)(['\"])?:", r'"\2": ', metadatastring)

    # restore only inode, directory, freeblock and superblock block others are just data blocks that we can ignore
    try:
      fileindex = int(filename.split('.')[1])
      if fileindex == 0:
        filesystemdatastructure['superBlock'] = json.loads(jsondata)
      elif fileindex >=1 and fileindex <= MAXFREEPOINTERBLOCKS:
        filesystemdatastructure['fblocks'][fileindex] = build_free_block_DS(metadatastring, fileindex) 

    except Exception as e:
      print e
      pass  
  build_inodetable(26)

# I'm already added.
def build_inodetable(inode):
  metadatafo = open(DATAPATH+'fusedata.'+str(inode),'r')
  filedata = metadatafo.read()
  metadatafo.close()

  jsondata = re.sub(r"(['\"])?([a-zA-Z0-9_.:]+)(['\"])?:", r'"\2": ', filedata)
  objdata = json.loads(jsondata)
  filesystemdatastructure['intab'][inode] = objdata
  # for each entry in my table...
  for entryname,entryinode in filesystemdatastructure['intab'][inode]['filename_to_inode_dict'].iteritems():
    typeof = entryname[0]
    # ignore the initial F,D prefix
    entryname = entryname[2:]
    # if it's . or .. skip it.
    if entryname == '.' or entryname == '..':
      continue

    # and recurse if a directory...
    if typeof == 'd':
      build_inodetable(entryinode)
    elif typeof == 'f':
      metadatafo = open(DATAPATH+'fusedata.'+str(entryinode),'r')
      filedata = metadatafo.read()
      metadatafo.close()

      jsondata = re.sub(r"(['\"])?([a-zA-Z0-9_.:]+)(['\"])?:", r'"\2": ', filedata)
      objdata = json.loads(jsondata)
      filesystemdatastructure['intab'][entryinode] = objdata
    

def test_all():

  currtime = int(time.time())

  numblocks=0

  for key, val in filesystemdatastructure['intab'].iteritems():
    #(2)check for all times in past
    if ( val['atime'] >= currtime or val['ctime'] >= currtime or val['mtime'] >= currtime ) :
      print("inode :"+str(key)+" has invalid time  atime or ctime or mtime")

    #(4, 5)check for directory link count and links consistancy
    if 'filename_to_inode_dict' in val  :
      if DIRPFIX+'.' not in val['filename_to_inode_dict'] or DIRPFIX+'..' not in val['filename_to_inode_dict'] :
        print("Directory inode :"+str(key)+" does not contain either . or ..")

      dirLinks = [k for k in val['filename_to_inode_dict'] if DIRPFIX in k ]
      linkcount = len(dirLinks)
      if filesystemdatastructure['intab'][key]['linkcount'] != linkcount:
        print("Directory inode :"+str(key)+" linkcount mismatch")

    #(6,7)check for indirect pointer and file size consistancy
    if 'location' in val  :
      fd = open(DATAPATH+'fusedata.'+str(val['location']), 'r')
      try:
        if not if_array(fd.read()):
          raise Exception("")
      except Exception :
        if val['indirect'] == 1:
          print("File inode :"+str(key)+" has indirect pointer set but index block missing")
        else:
          pass
      block_count =0
      if val['indirect'] == 1:
        data = fd.read()
        data = data.split(",")
        for k in data:
          block_count += 1

      #7.a
      if val['size'] > 0 and val['indirect'] == 0 and val['size'] > BLOCKSIZE:
        print("File inode :"+str(key)+" has indirect pointer set to 0 but file size exceeds block size")
      #7.b
      if val['indirect'] == 1 and val['size'] > BLOCKSIZE*(block_count):
        print("File inode :"+str(key)+" has indirect pointer set to 1 but file size exceeds block size*number of blocks")
      #7.c
      if val['indirect'] == 1 and val['size'] < BLOCKSIZE*(block_count)-1:
        print("File inode :"+str(key)+" has indirect pointer set to 1 but file size is short of block size*number of blocks")

  #(3.b)check for free block consistancy
  for key, val in filesystemdatastructure['fblocks'].iteritems():
    for k in val :
      numblocks += 1
      currinode = filesystemdatastructure['fblocks'][key][k]
      
      if currinode == True:   #free block
        size = 0
        try:
          size = os.path.getsize(DATAPATH+'fusedata.'+str(k))
        except Exception as e:
          pass
        if size != 0:
          print("Free block claims that the location (%d) is free but it contain a file" % k)
     
  #(3.a) freeblock list contains all blocks     
  if numblocks != 10000:
      print("Free Blocks do not add up to "+str(10000)+" blocks")

  #(1)check of device ID
  if filesystemdatastructure['superBlock']['devId'] != 20:
    print( "Device ID is not 20")

#helper function
def build_free_block_DS(data, op):
  fbs = data.split(",")
  map(str.strip, fbs)
  ret = {}
  intblockindex=0
  for block in fbs:
    intblockindex = int(block)
    ret[intblockindex] = True
  for i in range((op - 1)*400, op*400):
    if i not in ret:
      ret[i] = False;

  return ret

def if_array(data):
  try:
    nums = split(data)
  except Exception:   #if no , was found
    try:
      int(data)
      return True
    except ValueError:
      return False
  #loop through each term and confirm that s its an integer
  for num in nums:
    try:
      int(num)
    except ValueError:
      return False
  return True


if __name__ == '__main__':
  restore_FS()
  test_all()