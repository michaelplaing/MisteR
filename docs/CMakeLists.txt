set(DOXYGEN_EXTRACT_ALL YES)

doxygen_add_docs(
    docs
    ../include
    ../src
    ../module
)