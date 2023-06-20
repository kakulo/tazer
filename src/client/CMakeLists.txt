SET(TAZER_CLIENT_INC_DIR ${TAZER_INC_DIR})
set(CLIENT_HEADERS
    ${TAZER_CLIENT_INC_DIR}/TrackFile.h
    ${TAZER_CLIENT_INC_DIR}/AtomicHelper.h
    ${TAZER_CLIENT_INC_DIR}/FcntlReaderWriterLock.h
    ${TAZER_CLIENT_INC_DIR}/ThreadPool.h
    ${TAZER_CLIENT_INC_DIR}/BoundedCache.h
    ${TAZER_CLIENT_INC_DIR}/FileCache.h
    ${TAZER_CLIENT_INC_DIR}/Timer.h
    ${TAZER_CLIENT_INC_DIR}/Histogram.h
    ${TAZER_CLIENT_INC_DIR}/ScalableCache.h
    ${TAZER_CLIENT_INC_DIR}/ScalableMetaData.h
    ${TAZER_CLIENT_INC_DIR}/ScalableAllocator.h
    ${TAZER_CLIENT_INC_DIR}/FileCacheRegister.h
    ${TAZER_CLIENT_INC_DIR}/PerfectPrefetcher.h
    ${TAZER_CLIENT_INC_DIR}/Trackable.h
    ${TAZER_CLIENT_INC_DIR}/Cache.h
    ${TAZER_CLIENT_INC_DIR}/Prefetcher.h
    ${TAZER_CLIENT_INC_DIR}/UnboundedCache.h
    ${TAZER_CLIENT_INC_DIR}/CacheStats.h
    ${TAZER_CLIENT_INC_DIR}/FilelockCache.h
    ${TAZER_CLIENT_INC_DIR}/UnixIO.h
    ${TAZER_CLIENT_INC_DIR}/CacheTypes.h
    ${TAZER_CLIENT_INC_DIR}/Config.h
    ${TAZER_CLIENT_INC_DIR}/Loggable.h
    ${TAZER_CLIENT_INC_DIR}/PriorityThreadPool.h
    ${TAZER_CLIENT_INC_DIR}/Connection.h
    ${TAZER_CLIENT_INC_DIR}/MemoryCache.h
    ${TAZER_CLIENT_INC_DIR}/RSocketAdapter.h
    ${TAZER_CLIENT_INC_DIR}/ConnectionPool.h
    ${TAZER_CLIENT_INC_DIR}/Message.h
    ${TAZER_CLIENT_INC_DIR}/ReaderWriterLock.h
    ${TAZER_CLIENT_INC_DIR}/DeltaPrefetcher.h
    ${TAZER_CLIENT_INC_DIR}/NetworkCache.h
    ${TAZER_CLIENT_INC_DIR}/lz4.h
    ${TAZER_CLIENT_INC_DIR}/ReaderWriterLock.h
    ${TAZER_CLIENT_INC_DIR}/lz4hc.h
    ${TAZER_CLIENT_INC_DIR}/FcntlCache.h
    ${TAZER_CLIENT_INC_DIR}/SharedMemoryCache.h
    ${TAZER_CLIENT_INC_DIR}/xxhash.h
)

set(CLIENT_FILES
    InputFile.cpp
    OutputFileInner.cpp
    OutputFile.cpp
    LocalFile.cpp
    TrackFile.cpp
    TazerFileStream.cpp
    Lib.cpp
    PriorityThreadPool.cpp
    RSocketAdapter.cpp
    ThreadPool.cpp
    ReaderWriterLock.cpp
    Timer.cpp
    Connection.cpp
    Loggable.cpp
    Request.cpp
    ConnectionPool.cpp
    TazerFile.cpp
    Prefetcher.cpp
    TazerFileDescriptor.cpp
)

add_library(client SHARED ${CLIENT_FILES})
#target_link_libraries(client ${RDMACM_LIB} ${RT_LIB} ${CURL_LIB} ${SSL_LIB} stdc++fs) 
target_link_libraries(client stdc++fs) 
target_include_directories(client PUBLIC ${TAZER_INC_DIR})
install(TARGETS client LIBRARY DESTINATION lib)
 
