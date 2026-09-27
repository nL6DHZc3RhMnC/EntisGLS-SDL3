#!/usr/bin/env python3
"""Host JVM checks for production per-game settings and DLL import data handling.

Uses a fake provider/preferences store. Does not claim Android permission,
Activity lifecycle, or native PSB compatibility coverage.
"""

# Support direct execution from any working directory.
import sys as _sys
from pathlib import Path as _BootstrapPath
_sys.path.insert(0, str(next(parent / "tools" for parent in _BootstrapPath(__file__).resolve().parents
                             if (parent / "tools" / "_bootstrap.py").is_file())))
from _bootstrap import ROOT

from pathlib import Path
import subprocess
import tempfile

from java_runtime import find_jdk
JDK = find_jdk(ROOT)
STUBS = {
    'android/net/Uri.java': 'package android.net; public final class Uri {}',
    'android/os/ParcelFileDescriptor.java': 'package android.os; public class ParcelFileDescriptor implements AutoCloseable { public static ParcelFileDescriptor adoptFd(int fd){return new ParcelFileDescriptor();} public java.io.FileDescriptor getFileDescriptor(){throw new UnsupportedOperationException();} public void close(){} public static class AutoCloseOutputStream extends java.io.OutputStream { public AutoCloseOutputStream(ParcelFileDescriptor p){} public void write(int n){} } }',
    'io/entisgls/launcher/sdl/DocumentTreeAccess.java': 'package io.entisgls.launcher.sdl; import android.content.Context; import android.net.Uri; import java.io.IOException; public class DocumentTreeAccess { public DocumentTreeAccess(Context c,Uri u) throws IOException {} public long[] stat(String p) throws IOException {throw new IOException("stub");} public int open(String p,String m) throws IOException {throw new IOException("stub");} public void remove(String p,boolean d) throws IOException {throw new IOException("stub");} }',
    'android/database/Cursor.java': 'package android.database; public interface Cursor extends AutoCloseable { boolean moveToFirst(); int getColumnIndex(String name); String getString(int i); long getLong(int i); boolean isNull(int i); void close(); }',
    'android/content/SharedPreferences.java': 'package android.content; import java.util.Map; public interface SharedPreferences { Map<String,?> getAll(); Editor edit(); interface Editor { Editor putString(String k,String v); Editor remove(String k); boolean commit(); } }',
    'android/content/Context.java': 'package android.content; public abstract class Context { public static final int MODE_PRIVATE=0; public abstract SharedPreferences getSharedPreferences(String s,int mode); public abstract ContentResolver getContentResolver(); public java.io.File getCacheDir(){throw new UnsupportedOperationException();} }',
    'android/content/ContentResolver.java': 'package android.content; import android.database.Cursor; import android.net.Uri; import java.io.*; public abstract class ContentResolver { public abstract Cursor query(Uri u,String[] p,String a,String[] b,String c); public abstract InputStream openInputStream(Uri u) throws IOException; }',
    'android/provider/OpenableColumns.java': 'package android.provider; public class OpenableColumns { public static final String DISPLAY_NAME="name", SIZE="size"; }',
    'android/system/ErrnoException.java': 'package android.system; public class ErrnoException extends Exception { public ErrnoException(Throwable t){super(t);} }',
    'android/system/Os.java': 'package android.system; import java.nio.file.*; public class Os { public static void rename(String a,String b) throws ErrnoException { try{ Files.move(Paths.get(a),Paths.get(b),StandardCopyOption.REPLACE_EXISTING,StandardCopyOption.ATOMIC_MOVE); }catch(Exception e){throw new ErrnoException(e);} } }',
    'android/util/Log.java': 'package android.util; public class Log { public static int w(String t,String m,Throwable e){return 0;} }',
    'io/entisgls/launcher/sdl/ResourceStore.java': '''package io.entisgls.launcher.sdl; import android.content.Context; import java.io.*; final class ResourceStore { static Game fixture; static final class Game { final File directory; final android.net.Uri tree=null; Game(File d){directory=d;} } static Game game(Context c,String id) throws IOException { if(!validId(id))throw new IOException("id");return fixture; } static boolean validId(String id){return id!=null&&(id.equals("legacy")||id.matches("[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}"));} }''',
}
HARNESS = r'''
package io.entisgls.launcher.sdl;
import android.content.*;
import android.database.Cursor;
import android.net.Uri;
import java.io.*;
import java.nio.file.*;
import java.util.*;

public final class PsbSettingsTest {
 static int count;
 interface Checked {void run() throws Exception;}
 static void check(boolean condition,String label){if(!condition)throw new AssertionError(label);++count;}
 static void reject(Checked action) throws Exception {try{action.run();throw new AssertionError("Expected failure");}catch(IOException expected){++count;}}
 static class Prefs implements SharedPreferences {
  Map<String,String> values=new HashMap<>(); boolean fail;
  public Map<String,?> getAll(){return values;}
  public Editor edit(){return new Editor(){Map<String,String> next=new HashMap<>(values);public Editor putString(String k,String v){next.put(k,v);return this;}public Editor remove(String k){next.remove(k);return this;}public boolean commit(){if(fail)return false;values=next;return true;}};}
 }
 static class Provider extends ContentResolver {
  byte[] bytes; String name="original-driver.dll"; Long size;
  Provider set(byte[] b){bytes=b;size=(long)b.length;return this;}
  public Cursor query(Uri u,String[] p,String a,String[] b,String c){return new Cursor(){public boolean moveToFirst(){return true;}public int getColumnIndex(String n){return n.equals("name")?1:0;}public String getString(int i){if(i!=1)throw new AssertionError("column mapping");return name;}public long getLong(int i){return size;}public boolean isNull(int i){return size==null;}public void close(){}};}
  public InputStream openInputStream(Uri u){return new ByteArrayInputStream(bytes);}
 }
 static class Host extends Context {
  final Prefs prefs=new Prefs(); final Provider provider=new Provider();
  public SharedPreferences getSharedPreferences(String s,int m){return prefs;}
  public ContentResolver getContentResolver(){return provider;}
 }
 static byte[] pe(){byte[] b=new byte[128];b[0]='M';b[1]='Z';b[60]=64;b[64]='P';b[65]='E';b[87]=0x20;return b;}
 public static void main(String[] args) throws Exception {
  Host host=new Host(); final String first="legacy", second="12345678-1234-1234-1234-123456789abc";
  check(PsbKeySettings.normalize(" \t")==null,"empty automatic");
  check(PsbKeySettings.normalize("0").equals("0"),"explicit zero");
  check(PsbKeySettings.normalize("0XFFFFFFFF").equals("4294967295"),"hex upper maximum");
  check(PsbKeySettings.normalize("00012").equals("12"),"decimal leading zeros");
  for(String bad:new String[]{"-1","+1","4294967296","0x100000000","0x","1 2","1e3","１"})reject(()->PsbKeySettings.normalize(bad));
  reject(()->PsbKeySettings.write(host,"../outside","1"));
  check(PsbKeySettings.read(host,first)==null,"missing override");
  PsbKeySettings.write(host,first,"0xABC");PsbKeySettings.write(host,second,"73");
  check(PsbKeySettings.read(host,first).equals("2748"),"save and normalize");
  PsbKeySettings.write(host,first,"");
  check(PsbKeySettings.read(host,first)==null&&PsbKeySettings.read(host,second).equals("73"),"clear and isolate games");
  host.prefs.fail=true;reject(()->PsbKeySettings.write(host,second,"17"));host.prefs.fail=false;
  check(PsbKeySettings.read(host,second).equals("73"),"failed save keeps prior override");
  Path root=Files.createTempDirectory("entis-driver-test-").toFile().getCanonicalFile().toPath();
  try {
   ResourceStore.fixture=new ResourceStore.Game(Files.createDirectory(root.resolve("game")).toFile());
   Path destination=root.resolve("game/emotedriver.dll");
   host.provider.set(pe());Uri source=new Uri();
   EmoteDriverImport.copy(host,first,source);
   check(Arrays.equals(Files.readAllBytes(destination),pe()),"DLL copied without transformation");
   byte[] changed=pe();changed[120]=17;host.provider.set(changed);host.provider.size=null;
   EmoteDriverImport.copy(host,first,source);
   check(Arrays.equals(Files.readAllBytes(destination),changed),"unknown provider size copied and replaced");
   try(java.util.stream.Stream<Path> entries=Files.list(destination.getParent())){
    Path backup=entries.filter(p->p.getFileName().toString().startsWith(".emotedriver-backup-")).findFirst().orElseThrow(AssertionError::new);
    check(Arrays.equals(Files.readAllBytes(backup),pe()),"old driver backup preserved");
   }
   host.provider.set(new byte[]{1,2,3});reject(()->EmoteDriverImport.copy(host,first,source));
   host.provider.set(pe());host.provider.name="program.exe";reject(()->EmoteDriverImport.copy(host,first,source));host.provider.name="driver.dll";
   byte[] bad=pe();bad[87]=0;host.provider.set(bad);reject(()->EmoteDriverImport.copy(host,first,source));
   bad=pe();bad[60]=(byte)255;host.provider.set(bad);reject(()->EmoteDriverImport.copy(host,first,source));
   host.provider.set(pe());host.provider.size=999L;reject(()->EmoteDriverImport.copy(host,first,source));
   host.provider.size=257L*1024*1024;reject(()->EmoteDriverImport.copy(host,first,source));
   host.provider.set(pe());Thread.currentThread().interrupt();try{reject(()->EmoteDriverImport.copy(host,first,source));}finally{Thread.interrupted();}
   check(Arrays.equals(Files.readAllBytes(destination),changed),"all rejected copies preserve active driver");
   try(java.util.stream.Stream<Path> entries=Files.list(destination.getParent())){check(entries.noneMatch(p->p.getFileName().toString().endsWith(".tmp")),"failed copies clean staging files");}
   Path outside=root.resolve("outside.dll");Files.write(outside,new byte[]{5});Files.delete(destination);Files.createSymbolicLink(destination,outside);
   reject(()->EmoteDriverImport.copy(host,first,source));
   check(Arrays.equals(Files.readAllBytes(outside),new byte[]{5}),"linked destination never changes outside file");
  } finally {try(java.util.stream.Stream<Path> paths=Files.walk(root)){paths.sorted(Comparator.reverseOrder()).forEach(p->{try{Files.delete(p);}catch(IOException e){throw new RuntimeException(e);}});}}
  System.out.println("PASS "+count+" PSB settings/driver-copy checks (host JVM; no device/lifecycle claim)");
 }
}
'''

with tempfile.TemporaryDirectory(prefix='entis-psb-jvm-') as temp:
    root = Path(temp)
    for name, source in STUBS.items():
        path = root / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(source)
    harness = root / 'io/entisgls/launcher/sdl/PsbSettingsTest.java'
    harness.write_text(HARNESS)
    classes = root / 'classes'
    classes.mkdir()
    production = ROOT / 'apps/android/java/io/entisgls/launcher/sdl'
    subprocess.run([str(JDK / 'bin/javac'), '-encoding', 'UTF-8', '-d', str(classes),
                    *map(str, root.rglob('*.java')), str(production / 'PsbKeySettings.java'),
                    str(production / 'EmoteDriverImport.java')], check=True)
    subprocess.run([str(JDK / 'bin/java'), '-cp', str(classes),
                    'io.entisgls.launcher.sdl.PsbSettingsTest'], check=True)
