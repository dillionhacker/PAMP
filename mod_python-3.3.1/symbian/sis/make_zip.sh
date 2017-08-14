rm -rf epoc32

mkdir epoc32
mkdir epoc32/winscw
mkdir epoc32/winscw/c
mkdir epoc32/winscw/c/resource
mkdir epoc32/winscw/c/resource/mod_python

cp ../python-2.3.6/original/lib/new.py            epoc32/winscw/c/resource

cp ../python-2.2.3/original/lib/cgi.py            epoc32/winscw/c/resource
cp ../python-2.2.3/original/lib/hmac.py           epoc32/winscw/c/resource
cp ../python-2.2.3/original/lib/dis.py            epoc32/winscw/c/resource
cp ../python-2.2.3/original/lib/ftplib.py         epoc32/winscw/c/resource
cp ../python-2.2.3/original/lib/gopherlib.py      epoc32/winscw/c/resource
cp ../python-2.2.3/original/lib/inspect.py        epoc32/winscw/c/resource
cp ../python-2.2.3/original/lib/mimetypes.py      epoc32/winscw/c/resource
cp ../python-2.2.3/original/lib/posixpath.py      epoc32/winscw/c/resource
cp ../python-2.2.3/original/lib/token.py          epoc32/winscw/c/resource
cp ../python-2.2.3/original/lib/tokenize.py       epoc32/winscw/c/resource
cp ../python-2.2.3/original/lib/urllib2.py        epoc32/winscw/c/resource
cp ../python-2.2.3/modified/lib/pickle.py         epoc32/winscw/c/resource
cp ../python-2.2.3/modified/lib/tempfile.py       epoc32/winscw/c/resource

cp ../../lib/python/mod_python/Cookie.py          epoc32/winscw/c/resource/mod_python
cp ../../lib/python/mod_python/Session.py         epoc32/winscw/c/resource/mod_python
cp ../../lib/python/mod_python/__init__.py        epoc32/winscw/c/resource/mod_python
cp ../../lib/python/mod_python/apache.py          epoc32/winscw/c/resource/mod_python
cp ../../lib/python/mod_python/cgihandler.py      epoc32/winscw/c/resource/mod_python
cp ../../lib/python/mod_python/importer.py        epoc32/winscw/c/resource/mod_python
cp ../../lib/python/mod_python/psp.py             epoc32/winscw/c/resource/mod_python
cp ../../lib/python/mod_python/publisher.py       epoc32/winscw/c/resource/mod_python
cp ../../lib/python/mod_python/python22.py        epoc32/winscw/c/resource/mod_python
cp ../../lib/python/mod_python/util.py            epoc32/winscw/c/resource/mod_python
cp ../../lib/python/mod_python/testhandler.py     epoc32/winscw/c/resource/mod_python

cp ../../lib/python/mod_python/dummy_threading.py epoc32/winscw/c/resource/mod_python
cp ../../lib/python/mod_python/pdb.py             epoc32/winscw/c/resource/mod_python
cp ../../lib/python/mod_python/cPickle.py         epoc32/winscw/c/resource/mod_python
cp ../../lib/python/mod_python/cStringIO.py       epoc32/winscw/c/resource/mod_python

rm -f epoc32.zip

zip -r epoc32.zip epoc32
