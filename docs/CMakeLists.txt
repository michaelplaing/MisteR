set(DOXYGEN_EXTRACT_ALL YES)
set(DOXYGEN_EXTRACT_STATIC YES)

doxygen_add_docs(
    docs
    ../include
    ../src
    ../module
)