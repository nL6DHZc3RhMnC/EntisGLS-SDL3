#!/usr/bin/env python3
"""Install a project-local Android cross-build toolchain; verify downloads."""

# Support direct execution from any working directory.
import sys as _sys
from pathlib import Path as _BootstrapPath
_sys.path.insert(0, str(next(parent / "tools" for parent in _BootstrapPath(__file__).resolve().parents
                             if (parent / "tools" / "_bootstrap.py").is_file())))
from _bootstrap import ROOT

import concurrent.futures
import hashlib
import json
from pathlib import Path
import subprocess
import stat
import tarfile
import urllib.request
import xml.etree.ElementTree as ET
import zipfile

DEST = ROOT / '.android-tools'
BASE = 'https://dl.google.com/android/repository/'
PACKAGES = {
    'ndk;27.2.12479018': 'ndk',
    'build-tools;35.0.0': 'build-tools',
    'platforms;android-35': 'platform',
    'platform-tools': 'platform-tools',
}

def download(url, path, digest, algorithm):
    if path.exists() and hashlib.new(algorithm, path.read_bytes()).hexdigest() == digest:
        return
    partial = path.with_suffix(path.suffix + '.part')
    subprocess.run(['curl', '-fL', '--retry', '3', '--connect-timeout', '30',
                    '--silent', '--show-error', '-o', str(partial), url], check=True)
    actual = hashlib.new(algorithm, partial.read_bytes()).hexdigest()
    if actual != digest:
        raise RuntimeError(f'Checksum mismatch for {path.name}: {actual}')
    partial.replace(path)

def install_zip(key, dest, archive):
    target = DEST / dest
    if (target / '.complete').exists():
        print(f'Ready: {key}', flush=True)
        return
    url = BASE + archive.findtext('url')
    cache = DEST / 'downloads' / Path(url).name
    print(f'Downloading {key} ({int(archive.findtext("size")) // 1000000} MB)', flush=True)
    download(url, cache, archive.findtext('checksum'), 'sha1')
    target.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(cache) as z:
        for info in z.infolist():
            # SDK archives have one enclosing directory. Retain their layout.
            p = target / info.filename
            if not p.resolve().is_relative_to(target.resolve()):
                raise ValueError('Unsafe archive path')
            mode = info.external_attr >> 16
            if stat.S_ISLNK(mode):
                link = z.read(info).decode()
                if not (p.parent / link).resolve().is_relative_to(target.resolve()):
                    raise ValueError('Unsafe symlink target')
                p.parent.mkdir(parents=True, exist_ok=True)
                if p.exists() or p.is_symlink():
                    p.unlink()
                p.symlink_to(link)
                continue
            z.extract(info, target)
            if mode and not info.is_dir():
                p.chmod(mode & 0o777)
    (target / '.complete').write_text(url + '\n')
    print(f'Installed: {key}', flush=True)

def install_jdk():
    target = DEST / 'jdk'
    if (target / '.complete').exists():
        return
    request = urllib.request.Request(
        'https://api.github.com/repos/adoptium/temurin17-binaries/releases/latest',
        headers={'User-Agent': 'StudySteady-Android-Build'})
    data = json.load(urllib.request.urlopen(request, timeout=30))
    asset = next(a for a in data['assets'] if 'jdk_x64_mac_hotspot' in a['name']
                 and a['name'].endswith('.tar.gz'))
    checksum_url = asset['browser_download_url'] + '.sha256.txt'
    checksum = subprocess.check_output(['curl', '-fsSL', checksum_url], text=True).split()[0]
    cache = DEST / 'downloads' / asset['name']
    print('Downloading JDK 17', flush=True)
    download(asset['browser_download_url'], cache, checksum, 'sha256')
    target.mkdir(parents=True, exist_ok=True)
    with tarfile.open(cache) as tar:
        for entry in tar.getmembers():
            if not (target / entry.name).resolve().is_relative_to(target.resolve()):
                raise ValueError('Unsafe archive path')
        tar.extractall(target)
    (target / '.complete').write_text(asset['browser_download_url'] + '\n')
    print('Installed: JDK 17', flush=True)

def main():
    (DEST / 'downloads').mkdir(parents=True, exist_ok=True)
    xml = ET.fromstring(urllib.request.urlopen(BASE + 'repository2-3.xml', timeout=30).read())
    tasks = []
    with concurrent.futures.ThreadPoolExecutor(max_workers=3) as pool:
        tasks.append(pool.submit(install_jdk))
        for package in xml.findall('remotePackage'):
            key = package.attrib['path']
            if key not in PACKAGES:
                continue
            for archive in package.findall('./archives/archive'):
                if archive.findtext('host-os') in ('macosx', None):
                    tasks.append(pool.submit(install_zip, key, PACKAGES[key], archive.find('complete')))
                    break
        for task in tasks:
            task.result()

if __name__ == '__main__':
    main()
