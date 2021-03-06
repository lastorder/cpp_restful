basepath=$(cd `dirname $0`; pwd)


#if false; then
opensslfile=openssl-1.0.2l
tar -xzf ${opensslfile}.tar.gz
cd ${opensslfile}
./config --prefix=${basepath}/install --openssldir=${basepath}/install/openssl
make
make test
make install
#fi


cd $basepath

libeventfile=libevent-2.1.8-stable
tar -xzf ${libeventfile}.tar.gz
cd ${libeventfile}
./configure --prefix=$basepath/install --disable-libevent-regress --disable-samples CPPFLAGS="-fPIC -I${basepath}/install/include" LDFLAGS="-L${basepath}/install/lib"
make
make install

#fi

cd $basepath
mkdir linux
mkdir -p linux/include
mkdir -p linux/lib


cp -r install/include/event2 linux/include/event2
cp -r ${libeventfile}/compat/sys/queue.h linux/include/event2
cp -r install/include/openssl linux/include

cp install/lib/*.a linux/lib


