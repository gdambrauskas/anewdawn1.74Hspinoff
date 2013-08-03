'''

Script to run on 2 directories which produces a list of
files that are new or different in size. The list is
copied to a different directory and can be used as a
patch.

Created on Aug 1, 2013

@author: gvd
'''
import os
from distutils.dir_util import mkpath,copy_tree
import shutil

def compareDirectories(new, old, relpath = ""):
    log("XXX new " + new + " old "+old + " relpath "+relpath)
    newroot, newdirs, newfiles = next(os.walk(new))
    oldroot, olddirs, oldfiles = next(os.walk(old))
    log("new dirs: " + ",".join(newdirs))
    all_dirs = list(set(newdirs) | set(olddirs))
    log("all dirs: " + ",".join(all_dirs))
    common_dirs = list(set(newdirs) & set(olddirs))
    log("common dirs: "+ ",".join(common_dirs))
    missing_new_dirs = list(set(newdirs) - set(common_dirs))
    log("missing new dirs: "+ ",".join(missing_new_dirs))
    compareFiles(newroot,oldroot,newfiles, oldfiles, relpath)    
    for missing_dir in missing_new_dirs:
        full_path = os.path.join(diff_root,relpath)
        full_path = os.path.join(full_path,missing_dir)
        log("full " + full_path)
        mkpath(full_path)
        src = os.path.join(newroot,missing_dir)
        log("missing dir src "+src)
        copy_tree(src, full_path)
    for common_dir in common_dirs:
        next_new_dir = os.path.join(newroot,common_dir)
        next_old_dir = os.path.join(oldroot,common_dir)
        newrelpath = os.path.join(relpath,common_dir);
        log("newrelpath "+newrelpath)
        compareDirectories(next_new_dir, next_old_dir, newrelpath)        
        
def compareFiles(newroot,oldroot,newfiles, oldfiles, relpath):
    log("compare files in "+newroot)
    common_files = list(set(newfiles) & set(oldfiles))
    missing_new_files = list(set(newfiles) - set(common_files))
    for file in missing_new_files:
        full_path = os.path.join(diff_root,relpath)
        mkpath(full_path)    
        src = os.path.join(newroot,file)
        shutil.copy(src, full_path)
    for file in common_files:
        newfile = os.path.join(newroot,file)
        oldfile = os.path.join(oldroot,file)
        if os.path.getsize(newfile) != os.path.getsize(oldfile):
            full_path = os.path.join(diff_root,relpath)
            mkpath(full_path)
            shutil.copy(newfile, full_path)

def log(message):
    if logging:
        print(message)

logging = False
civdiff = "W:\\temp\civdiff"
#new = "W:\\temp\\new"
new = "W:\games\civ4\Beyond the Sword\Mods\gandtestnew"
#old = "W:\\temp\old"
old = "W:\games\civ4\Beyond the Sword\Mods\gandold"

diff_root, diff_dirs, diff_files =  next(os.walk(civdiff))
compareDirectories(new, old)  