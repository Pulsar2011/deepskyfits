# Downloads URL into OUT, through OUT.part so that an interrupted download never leaves a
# truncated file that the build would take as up to date.
#   cmake -DURL=<url> -DOUT=<file> -P DownloadFile.cmake
if(NOT URL OR NOT OUT)
    message(FATAL_ERROR "usage: cmake -DURL=<url> -DOUT=<file> -P DownloadFile.cmake")
endif()

file(DOWNLOAD "${URL}" "${OUT}.part" STATUS _status TLS_VERIFY ON)
list(GET _status 0 _code)
list(GET _status 1 _message)
if(NOT _code EQUAL 0)
    file(REMOVE "${OUT}.part")
    message(FATAL_ERROR "Download of ${URL} failed: ${_message}")
endif()
file(RENAME "${OUT}.part" "${OUT}")
