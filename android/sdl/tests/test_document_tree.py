#!/usr/bin/env python3
"""Exercise the production SAF filesystem with opaque IDs, injected failures and restart recovery.

Host provider tests do not claim Android permission UI or real provider/descriptor coverage.
"""
from pathlib import Path
import subprocess
import tempfile
ROOT = Path(__file__).resolve().parents[3]
from java_runtime import find_jdk
JDK = find_jdk(ROOT)
STUBS = {
'android/net/Uri.java': '''package android.net; public final class Uri { public final String value; public Uri(String s){value=s;} public String getScheme(){return value.startsWith("content:")?"content":"doc";} public String toString(){return value;} public boolean equals(Object o){return o instanceof Uri&&value.equals(((Uri)o).value);} public int hashCode(){return value.hashCode();} }''',
'android/content/UriPermission.java': '''package android.content; import android.net.Uri; public class UriPermission { final Uri uri; final boolean r,w; public UriPermission(Uri u,boolean r,boolean w){uri=u;this.r=r;this.w=w;} public Uri getUri(){return uri;}public boolean isReadPermission(){return r;}public boolean isWritePermission(){return w;} }''',
'android/content/Context.java': '''package android.content; import java.io.File; public abstract class Context {public Context getApplicationContext(){return this;} public abstract ContentResolver getContentResolver();public abstract File getFilesDir();}''',
'android/database/Cursor.java': '''package android.database;public interface Cursor extends AutoCloseable {boolean moveToFirst();boolean moveToNext();String getString(int i);long getLong(int i);boolean isNull(int i);void close();}''',
'android/content/ContentResolver.java': '''package android.content; import android.database.Cursor;import android.net.Uri;import android.os.ParcelFileDescriptor;import java.util.List;import java.io.IOException;public abstract class ContentResolver {public abstract List<UriPermission> getPersistedUriPermissions();public abstract Cursor query(Uri u,String[] p,String a,String[] b,String c);public abstract ParcelFileDescriptor openFileDescriptor(Uri u,String mode)throws IOException;public abstract Uri create(Uri parent,String mime,String name)throws IOException;public abstract boolean delete(Uri u)throws IOException;public abstract Uri rename(Uri u,String name)throws IOException;}''',
'android/provider/DocumentsContract.java': '''package android.provider;import android.content.ContentResolver;import android.net.Uri;import java.io.IOException;public class DocumentsContract {public static class Document {public static final String COLUMN_DOCUMENT_ID="id",COLUMN_DISPLAY_NAME="name",COLUMN_MIME_TYPE="mime",COLUMN_SIZE="size",COLUMN_LAST_MODIFIED="modified",COLUMN_FLAGS="flags",MIME_TYPE_DIR="dir";public static final int FLAG_DIR_SUPPORTS_CREATE=8,FLAG_SUPPORTS_WRITE=2;}public static boolean isTreeUri(Uri u){return u.value.startsWith("content:tree:");}public static String getTreeDocumentId(Uri u){return u.value.substring(13);}public static Uri buildDocumentUriUsingTree(Uri t,String id){return new Uri("doc:"+id);}public static Uri buildChildDocumentsUriUsingTree(Uri t,String id){return new Uri("children:"+id);}public static Uri createDocument(ContentResolver r,Uri p,String m,String n)throws IOException{return r.create(p,m,n);}public static boolean deleteDocument(ContentResolver r,Uri u)throws IOException{return r.delete(u);}public static Uri renameDocument(ContentResolver r,Uri u,String n)throws IOException{return r.rename(u,n);}}''',
'android/os/ParcelFileDescriptor.java': '''package android.os;import java.io.FileDescriptor;import java.util.IdentityHashMap;public class ParcelFileDescriptor implements AutoCloseable {public static final IdentityHashMap<FileDescriptor,Boolean> seekable=new IdentityHashMap<>();public static int closed,detached;final FileDescriptor fd=new FileDescriptor();boolean transferred;public ParcelFileDescriptor(boolean s){seekable.put(fd,s);}public FileDescriptor getFileDescriptor(){return fd;}public int detachFd(){transferred=true;++detached;return 42;}public void close(){if(!transferred)++closed;seekable.remove(fd);}}''',
'android/system/ErrnoException.java': '''package android.system;public class ErrnoException extends Exception {public ErrnoException(Throwable t){super(t);}}''',
'android/system/OsConstants.java': '''package android.system;public class OsConstants {public static final int SEEK_CUR=1,SEEK_END=2;}''',
'android/system/Os.java': '''package android.system;import java.io.FileDescriptor;import java.nio.file.*;import android.os.ParcelFileDescriptor;public class Os {public static long lseek(FileDescriptor f,long o,int w)throws ErrnoException{if(!Boolean.TRUE.equals(ParcelFileDescriptor.seekable.get(f)))throw new ErrnoException(new Exception("pipe"));return w==2?99:0;}public static void rename(String a,String b)throws ErrnoException{try{Files.move(Paths.get(a),Paths.get(b),StandardCopyOption.ATOMIC_MOVE,StandardCopyOption.REPLACE_EXISTING);}catch(Exception e){throw new ErrnoException(e);}}}''',
'android/util/Log.java': '''package android.util;public class Log {public static int w(String t,String m,Throwable e){return 0;}}''',
}
HARNESS = r'''
package io.entisgls.launcher.sdl;
import android.content.*;import android.database.Cursor;import android.net.Uri;import android.os.ParcelFileDescriptor;import java.io.*;import java.nio.file.*;import java.util.*;
public class DocumentTreeTest {
 static int count;interface Checked{void run()throws Exception;}
 static void check(boolean p,String m){if(!p)throw new AssertionError(m);++count;}
 static void rejects(Checked f)throws Exception{try{f.run();throw new AssertionError("accepted");}catch(IOException expected){++count;}}
 static final Uri TREE=new Uri("content:tree:root");
 static class Node {String id,name,parent;boolean dir,seek=true;long size=73,flags=10;Node(String i,String n,String p,boolean d){id=i;name=n;parent=p;dir=d;}}
 static class Provider extends ContentResolver {
  final Map<String,Node> nodes=new LinkedHashMap<>();List<UriPermission> grants=new ArrayList<>();int sequence,renameCalls,failRename,crashRename;boolean createPipe,deny;
  Provider(){nodes.put("root",new Node("root","Game",null,true));grants.add(new UriPermission(TREE,true,true));}
  Node add(String name,String parent,boolean dir){Node n=new Node("opaque:"+(++sequence),name,parent,dir);nodes.put(n.id,n);return n;}
  Node named(String name){for(Node n:nodes.values())if(n.name.equals(name))return n;return null;}
  public List<UriPermission> getPersistedUriPermissions(){return grants;}
  public Cursor query(Uri u,String[] p,String a,String[] b,String c){
   if(deny)throw new SecurityException("revoked");List<Node> rows=new ArrayList<>();
   if(u.value.startsWith("children:")){String id=u.value.substring(9);for(Node n:nodes.values())if(id.equals(n.parent))rows.add(n);}
   else {Node n=nodes.get(u.value.substring(4));if(n!=null)rows.add(n);}
   return new Cursor(){int i=-1;public boolean moveToFirst(){i=0;return !rows.isEmpty();}public boolean moveToNext(){return ++i<rows.size();}public String getString(int j){Node n=rows.get(i);return j==0?n.id:j==1?n.name:n.dir?"dir":"file";}public long getLong(int j){return j==3?rows.get(i).size:j==5?rows.get(i).flags:10;}public boolean isNull(int j){return false;}public void close(){}};
  }
  public ParcelFileDescriptor openFileDescriptor(Uri u,String mode)throws IOException{Node n=nodes.get(u.value.substring(4));if(n==null)throw new FileNotFoundException();return new ParcelFileDescriptor(n.seek);}
  public Uri create(Uri parent,String mime,String name){Node n=add(name,parent.value.substring(4),mime.equals("dir"));n.seek=!createPipe;return new Uri("doc:"+n.id);}
  public boolean delete(Uri u){return nodes.remove(u.value.substring(4))!=null;}
  public Uri rename(Uri u,String name)throws IOException{
   ++renameCalls;if(renameCalls==failRename)throw new IOException("injected rename failure");
   Node n=nodes.get(u.value.substring(4));if(n==null)throw new IOException("missing rename");
   for(Node other:nodes.values())if(other!=n&&Objects.equals(n.parent,other.parent)&&other.name.equals(name))throw new IOException("exists");
   n.name=name;
   // Model providers whose document ID changes after every rename.
   nodes.remove(n.id);n.id="opaque:"+(++sequence);nodes.put(n.id,n);
   if(renameCalls==crashRename)throw new SimulatedDeath();
   return new Uri("doc:"+n.id);
  }
 }
 static class SimulatedDeath extends Error{}
 static class Host extends Context {final Path root;final Provider provider;Host(Path p,Provider r){root=p;provider=r;}public File getFilesDir(){return root.toFile();}public ContentResolver getContentResolver(){return provider;}}
 static DocumentTreeAccess access(Path root,Provider p)throws IOException{return new DocumentTreeAccess(new Host(root,p),TREE);}
 public static void main(String[] args)throws Exception{
  Path root=Files.createTempDirectory("entis-saf-");
  try {
   Provider p=new Provider();Node archive=p.add("Script.noa","root",false);Node folder=p.add("字型","root",true);p.add("Font.otf",folder.id,false);
   DocumentTreeAccess fs=access(root,p);
   check(fs.stat("")[0]==2&&fs.stat("script.NOA")[1]==73,"opaque ids and case-insensitive lookup");
   check(Arrays.asList(fs.list("字型")).contains("Font.otf"),"unicode directory");
   check(fs.stat("missing")[0]==0,"missing stat");
   check(fs.stat("")[1]==0,"directory size normalized to zero");
   archive.size=-1;check(fs.stat("Script.noa")[1]==99,"unknown provider length measured without copying");archive.size=73;
   for(String bad:new String[]{"/absolute","../out","a/../b","a\\b","a//b","a/",".","x\0y"})rejects(()->fs.stat(bad));
   check(fs.open("Script.noa","r")==42&&ParcelFileDescriptor.detached==1,"descriptor ownership transferred");
   int closedBefore=ParcelFileDescriptor.closed;archive.seek=false;rejects(()->fs.open("Script.noa","r"));check(ParcelFileDescriptor.closed==closedBefore+1,"pipe descriptor closed");archive.seek=true;
   rejects(()->fs.open("missing","r"));rejects(()->fs.open("missing","rw"));rejects(()->fs.open("Script.noa","bad"));
   fs.mkdir("savedata/deep");check(fs.stat("savedata/deep")[0]==2,"recursive mkdir");
   fs.open("savedata/deep/new.dat","rwt");check(fs.stat("savedata/deep/new.dat")[0]==1,"write creates document");
   rejects(()->fs.remove("savedata/deep",true));rejects(()->fs.remove("",true));
   fs.remove("savedata/deep/new.dat",false);fs.remove("savedata/deep",true);check(fs.stat("savedata/deep")[0]==0,"only empty directories deleted");
   p.createPipe=true;rejects(()->fs.open("new-pipe","rwt"));check(p.named("new-pipe")==null,"failed new descriptor cleans only created file");p.createPipe=false;
   Node duplicate=p.add("SCRIPT.NOA","root",false);rejects(()->fs.list(""));p.nodes.remove(duplicate.id);
   Node latin=p.add("I.dat","root",false),dotless=p.add("ı.dat","root",false);rejects(()->fs.stat("i.dat"));p.nodes.remove(latin.id);p.nodes.remove(dotless.id);
   p.deny=true;rejects(()->fs.stat("Script.noa"));p.deny=false;
   p.grants=Arrays.asList(new UriPermission(TREE,true,false));rejects(()->access(root,p));p.grants=Arrays.asList(new UriPermission(TREE,true,true));
   archive.name="changed.noa";check(fs.stat("Script.noa")[0]==0&&fs.stat("changed.noa")[0]==1,"no stale provider name cache");
   Node source=p.add("stage.dat","root",false);Node target=p.add("save.dat","root",false);source.size=101;target.size=55;
   fs.rename("stage.dat","save.dat");check(fs.stat("save.dat")[1]==101&&p.named("stage.dat")==null,"replacement with changing opaque ids");
   check(p.nodes.values().stream().noneMatch(n->n.name.startsWith(".entis-save-backup-")),"confirmed replacement cleans predecessor");
   p.add("stage.dat","root",false).size=202;p.failRename=p.renameCalls+2;
   rejects(()->fs.rename("stage.dat","save.dat"));check(fs.stat("save.dat")[1]==101,"failed publication rolls prior save back");p.failRename=0;
   // Death after moving the old target, before moving the staged replacement.
   p.crashRename=p.renameCalls+1;try{fs.rename("stage.dat","save.dat");throw new AssertionError("missing death");}catch(SimulatedDeath expected){++count;}
   p.crashRename=0;fs=access(root,p);check(fs.stat("save.dat")[1]==101,"startup restores prior save from interrupted backup");
   // Death after publishing replacement but before clearing the journal.
   p.crashRename=p.renameCalls+2;try{fs.rename("stage.dat","save.dat");throw new AssertionError("missing death");}catch(SimulatedDeath expected){++count;}
   p.crashRename=0;fs=access(root,p);check(fs.stat("save.dat")[1]==202,"startup preserves published replacement");
   check(p.nodes.values().stream().anyMatch(n->n.name.startsWith(".entis-save-backup-")&&n.size==101),"ambiguous interrupted completion retains predecessor");
   final DocumentTreeAccess ready=fs;rejects(()->ready.rename("save.dat","savedata/save.dat"));
   System.out.println("PASS "+count+" SAF filesystem/transaction checks (host fake provider; Android device testing still required)");
  } finally {try(java.util.stream.Stream<Path> paths=Files.walk(root)){paths.sorted(Comparator.reverseOrder()).forEach(p->{try{Files.delete(p);}catch(IOException e){throw new RuntimeException(e);}});}}
 }
}
'''
# Lambdas capture the same instance until the explicit restart checks.
HARNESS = HARNESS.replace('DocumentTreeAccess fs=access(root,p);', 'final DocumentTreeAccess fs=access(root,p);')
HARNESS = HARNESS.replace('p.crashRename=0;fs=access(root,p);check(fs.stat("save.dat")[1]==101', 'p.crashRename=0;DocumentTreeAccess restarted=access(root,p);check(restarted.stat("save.dat")[1]==101')
HARNESS = HARNESS.replace('p.crashRename=0;fs=access(root,p);check(fs.stat("save.dat")[1]==202', 'p.crashRename=0;DocumentTreeAccess completed=access(root,p);check(completed.stat("save.dat")[1]==202')
with tempfile.TemporaryDirectory(prefix='entis-saf-jvm-') as directory:
    root=Path(directory)
    for name, source in STUBS.items():
        path=root/name; path.parent.mkdir(parents=True,exist_ok=True);path.write_text(source)
    harness=root/'io/entisgls/launcher/sdl/DocumentTreeTest.java'
    harness.parent.mkdir(parents=True,exist_ok=True);harness.write_text(HARNESS)
    classes=root/'classes';classes.mkdir()
    subprocess.run([str(JDK/'bin/javac'),'-encoding','UTF-8','-d',str(classes),*map(str,root.rglob('*.java')),str(ROOT/'android/sdl/java/io/entisgls/launcher/sdl/DocumentTreeAccess.java')],check=True)
    subprocess.run([str(JDK/'bin/java'),'-cp',str(classes),'io.entisgls.launcher.sdl.DocumentTreeTest'],check=True)
