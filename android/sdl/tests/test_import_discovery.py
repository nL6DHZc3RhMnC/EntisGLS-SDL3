#!/usr/bin/env python3
"""Run production importer discovery/path checks with a fake Android provider.

This tests provider input handling on the host. Android permission behavior,
Activity lifecycle, JSON transactions and SAF/device I/O still need device tests.
"""
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[3]
from java_runtime import find_jdk
JDK = find_jdk(ROOT)
STUBS = {
 'android/net/Uri.java': 'package android.net; public final class Uri { public final String id; public Uri(String id){this.id=id;} public static Uri parse(String s){return new Uri(s);} }',
 'android/os/ParcelFileDescriptor.java': 'package android.os; public class ParcelFileDescriptor implements AutoCloseable { public static ParcelFileDescriptor adoptFd(int fd){return new ParcelFileDescriptor();} public void close(){} }',
 'io/entisgls/launcher/sdl/DocumentTreeAccess.java': 'package io.entisgls.launcher.sdl; import android.content.Context; import android.net.Uri; import java.io.IOException; public class DocumentTreeAccess { public DocumentTreeAccess(Context c,Uri u) throws IOException {} public String[] list(String p) throws IOException {throw new IOException("stub");} public long[] stat(String p) throws IOException {throw new IOException("stub");} public int open(String p,String m) throws IOException {throw new IOException("stub");} public String displayName(){return "Fixture";} }',
 'android/database/Cursor.java': 'package android.database; public interface Cursor extends AutoCloseable { boolean moveToNext(); boolean moveToFirst(); String getString(int i); long getLong(int i); boolean isNull(int i); void close(); }',
 'android/content/ContentResolver.java': '''package android.content; import android.database.Cursor; import android.net.Uri; import java.io.*; public abstract class ContentResolver { public abstract Cursor query(Uri u,String[] p,String a,String[] b,String c); public InputStream openInputStream(Uri u) throws IOException { throw new IOException("not implemented in discovery fixture"); } }''',
 'android/content/Context.java': '''package android.content; import java.io.File; public abstract class Context { public abstract File getExternalFilesDir(String s); public abstract File getFilesDir(); public Context getApplicationContext(){return this;} public abstract ContentResolver getContentResolver(); }''',
 'android/os/Looper.java': 'package android.os; public class Looper { public static Looper getMainLooper(){return new Looper();} }',
 'android/os/Handler.java': 'package android.os; public class Handler { public Handler(Looper l){} public void post(Runnable r){r.run();} }',
 'android/os/StatFs.java': 'package android.os; public class StatFs { public StatFs(String p){} public long getAvailableBytes(){return Long.MAX_VALUE;} }',
 'android/os/SystemClock.java': 'package android.os; public class SystemClock { public static long uptimeMillis(){return System.currentTimeMillis();} }',
 'android/os/Process.java': 'package android.os; public class Process { public static final int THREAD_PRIORITY_BACKGROUND=10; public static void setThreadPriority(int i){} }',
 'android/system/ErrnoException.java': 'package android.system; public class ErrnoException extends Exception {}',
 'android/system/Os.java': 'package android.system; public class Os { public static void chmod(String p,int m) throws ErrnoException {} }',
 'android/util/Log.java': 'package android.util; public class Log { public static int w(String t,String m,Throwable e){return 0;} }',
 'android/provider/DocumentsContract.java': '''package android.provider; import android.net.Uri; public class DocumentsContract { public static class Document { public static final String COLUMN_DOCUMENT_ID="id",COLUMN_DISPLAY_NAME="name",COLUMN_SIZE="size",COLUMN_MIME_TYPE="mime",MIME_TYPE_DIR="dir"; } public static String getTreeDocumentId(Uri u){return u.id;} public static Uri buildChildDocumentsUriUsingTree(Uri t,String id){return new Uri("children:"+id);} public static Uri buildDocumentUriUsingTree(Uri t,String id){return new Uri(id);} }''',
 # Discovery tests must never enter JSON-dependent paths; fail loudly if they do.
 'org/json/JSONObject.java': '''package org.json; public class JSONObject { public JSONObject(){throw new UnsupportedOperationException();} public JSONObject(String s){throw new UnsupportedOperationException();} public JSONObject put(String k,Object v){throw new UnsupportedOperationException();} public String getString(String k){throw new UnsupportedOperationException();} public String optString(String k){throw new UnsupportedOperationException();} public String optString(String k,String d){throw new UnsupportedOperationException();} public int getInt(String k){throw new UnsupportedOperationException();} public String toString(int i){throw new UnsupportedOperationException();} }''',
 'org/json/JSONArray.java': 'package org.json; public class JSONArray { public JSONArray(){throw new UnsupportedOperationException();} public JSONArray put(Object v){throw new UnsupportedOperationException();} }',
}
HARNESS = r'''
package io.entisgls.launcher.sdl;
import android.content.*;
import android.database.Cursor;
import android.net.Uri;
import java.io.*;
import java.lang.reflect.*;
import java.nio.file.*;
import java.util.*;
import java.util.concurrent.CancellationException;
import java.util.concurrent.atomic.AtomicBoolean;

public final class ImportDiscoveryTest {
 static int count;
 static final class Row {
  String id,name,mime; Long size;
  Row(String id,String name,Long size,boolean dir){this.id=id;this.name=name;this.size=size;this.mime=dir?"dir":"file";}
 }
 static Row file(String name,long size){return new Row(name,name,size,false);}
 static Row dir(String id,String name){return new Row(id,name,0L,true);}
 static final class Rows implements Cursor {
  final List<Row> values; int n=-1;
  Rows(List<Row> rows){values=rows;}
  public boolean moveToNext(){return ++n<values.size();}
  public boolean moveToFirst(){n=0;return !values.isEmpty();}
  public String getString(int i){Row r=values.get(n);return i==0?r.id:i==1?r.name:r.mime;}
  public long getLong(int i){return values.get(n).size;}
  public boolean isNull(int i){return values.get(n).size==null;}
  public void close(){}
 }
 static final class Provider extends ContentResolver {
  final Map<String,List<Row>> folders=new HashMap<>();
  Provider root(Row...rows){folders.put("root",Arrays.asList(rows));return this;}
  Provider folder(String id,Row...rows){folders.put(id,Arrays.asList(rows));return this;}
  public Cursor query(Uri u,String[] p,String a,String[] b,String c){
   List<Row> rows=folders.get(u.id.substring("children:".length()));
   return rows==null?null:new Rows(rows);
  }
 }
 static Map<?,?> discover(Provider provider,boolean canceled) throws Exception {
  Method m=ResourceImporter.class.getDeclaredMethod("discover",ContentResolver.class,Uri.class,AtomicBoolean.class);
  m.setAccessible(true);
  try{return (Map<?,?>)m.invoke(null,provider,new Uri("root"),new AtomicBoolean(canceled));}
  catch(InvocationTargetException e){throw (Exception)e.getCause();}
 }
 static void check(boolean yes,String name){if(!yes)throw new AssertionError(name);++count;}
 static void rejected(Provider p,String name) throws Exception {
  try{discover(p,false);throw new AssertionError(name+" accepted");}catch(IOException expected){++count;}
 }
 public static void main(String[] args) throws Exception {
  Provider p=new Provider().root(file("Game.EXE",23),file("SCRIPT.NOA",40),dir("fonts","字型"),file("empty.dat",0))
    .folder("fonts",file("Font.otf",13),dir("deep","deep")).folder("deep",file("loose.csx",4));
  Map<?,?> found=discover(p,false);
  check(found.size()==7 && found.containsKey("字型/deep/loose.csx") && found.containsKey("empty.dat"),"recursive names and empty files");
  check(discover(new Provider().root(file("something.noa",3)),false).size()==1,"NOA candidate does not depend on game name");
  check(discover(new Provider().root(new Row("c","cotopha.xml",null,false)),false).size()==1,"unknown size source accepted");
  rejected(new Provider().root(file("readme.txt",3)),"no configuration candidate");
  rejected(new Provider().root(file("cotopha.xml",0)),"empty candidate");
  rejected(new Provider().root(file("game.exe",2),file("../bad",3)),"parent traversal");
  rejected(new Provider().root(file("game.exe",2),file("bad\\name",3)),"Windows separator");
  rejected(new Provider().root(file("game.exe",2),file("a/b",3)),"directory separator");
  rejected(new Provider().root(file("game.exe",2),file("x\0y",3)),"NUL filename");
  rejected(new Provider().root(file("game.exe",2),file("font.otf",3),file("FONT.OTF",2)),"case collision");
  rejected(new Provider().root(file("game.exe",2),dir("root","again")),"cyclic provider");
  try{discover(p,true);throw new AssertionError("cancellation ignored");}catch(CancellationException expected){++count;}
  Provider deep=new Provider().root(file("game.exe",3),dir("d0","d0"));
  for(int i=0;i<66;i++)deep.folder("d"+i,dir("d"+(i+1),"d"+(i+1)));
  rejected(deep,"bounded recursion");
  check(ResourceStore.validId("legacy") && ResourceStore.validId("021caa80-0741-49d9-b06f-d977afef927d"),"library IDs accepted");
  check(!ResourceStore.validId("../game") && !ResourceStore.validId(null),"library ID traversal rejected");
  Path base=Files.createTempDirectory("entis-import-fixture-");
  try {
   Path game=Files.createDirectory(base.resolve("game"));
   Files.write(game.resolve("cotopha.xml"),new byte[]{1});
   Files.createDirectories(game.resolve("fonts/deep"));
   Files.write(game.resolve("fonts/deep/font.otf"),new byte[]{2});
   Files.createFile(game.resolve("empty.dat"));
   Context context=new Context(){public File getExternalFilesDir(String s){return base.toFile();}public File getFilesDir(){return base.resolve("local").toFile();}public ContentResolver getContentResolver(){return p;}};
   ResourceStore.Game entry=new ResourceStore.Game("021caa80-0741-49d9-b06f-d977afef927d","Fixture",game.toFile(),false);
   check(ResourceStore.readiness(context,entry)==null,"recursive app readability and empty ancillary file");
   Path outside=Files.createDirectory(base.resolve("outside"));
   Files.write(outside.resolve("keep.dat"),new byte[]{3});
   Files.createSymbolicLink(game.resolve("linked"),outside);
   check(ResourceStore.readiness(context,entry)!=null,"linked resources rejected");
   Method remove=ResourceImporter.class.getDeclaredMethod("removeCreatedTree",File.class);remove.setAccessible(true);
   remove.invoke(null,game.toFile());
   check(Files.exists(outside.resolve("keep.dat")) && !Files.exists(game),"staging cleanup never follows outside link");
  } finally {
   try(java.util.stream.Stream<Path> paths=Files.walk(base)){paths.sorted(Comparator.reverseOrder()).forEach(path->{try{Files.deleteIfExists(path);}catch(IOException e){throw new RuntimeException(e);}});}
  }
  System.out.println("PASS "+count+" Android importer discovery/path tests (host fake provider; no device or transaction claim)");
 }
}
'''

with tempfile.TemporaryDirectory(prefix='entis-import-jvm-') as temp:
    root=Path(temp)
    for name,source in STUBS.items():
        path=root/name; path.parent.mkdir(parents=True,exist_ok=True); path.write_text(source)
    production=ROOT/'android/sdl/java/io/entisgls/launcher/sdl'
    harness=root/'io/entisgls/launcher/sdl/ImportDiscoveryTest.java'
    harness.parent.mkdir(parents=True,exist_ok=True); harness.write_text(HARNESS)
    classes=root/'classes'; classes.mkdir()
    subprocess.run([str(JDK/'bin/javac'),'-encoding','UTF-8','-d',str(classes),
                    *map(str,root.rglob('*.java')),str(production/'ResourceStore.java'),str(production/'ResourceImporter.java')],check=True)
    subprocess.run([str(JDK/'bin/java'),'-cp',str(classes),'io.entisgls.launcher.sdl.ImportDiscoveryTest'],check=True)
