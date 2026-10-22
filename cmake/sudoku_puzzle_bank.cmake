# The Sudoku Exchange puzzle bank whole (puzzles/sudoku_exchange/README.md), which the app offers new puzzles from: its
# four files at the commit the repository's sample was taken from, each held to its SHA-256, fetched once into the
# fetches' folder (FETCHCONTENT_BASE_DIR, kept between builds on CI) and read from there after. Sets
# SUDOKU_PUZZLE_BANK_FILES to the four files' paths.

include(FetchContent) # for FETCHCONTENT_BASE_DIR

set(SUDOKU_PUZZLE_BANK_COMMIT d8c8ebaee0c08c412cfba96af1923dfa61c83317)
set(sudoku_puzzle_bank_bands easy medium hard diabolical)
set(sudoku_puzzle_bank_hashes
    789aab6f52cc4588e0c3aa4269ad3282cfda7e1cc79ea693d16507decde3fc50
    c2a5f8fac99dcf215b25d46ece47f346375e51730b182de70a6f8f2107b2b38d
    abcff1512411e601abd5e861c60add5be5d877739b6faa249ee86960f5ad85e9
    08553d0c1145ea4d7c13008040f47ea8205d21fe1eaf8f4ab17a1a6981928b35)

set(sudoku_puzzle_bank_dir "${FETCHCONTENT_BASE_DIR}/sudoku-exchange-${SUDOKU_PUZZLE_BANK_COMMIT}")
set(SUDOKU_PUZZLE_BANK_FILES)
foreach(band hash IN ZIP_LISTS sudoku_puzzle_bank_bands sudoku_puzzle_bank_hashes)
    set(file "${sudoku_puzzle_bank_dir}/${band}.txt")
    set(is_current FALSE)
    if(EXISTS "${file}")
        file(SHA256 "${file}" actual)
        if(actual STREQUAL hash)
            set(is_current TRUE)
        endif()
    endif()
    if(NOT is_current)
        # Fetched beside the file and moved over it whole, so a broken download never stands as the file.
        message(STATUS "Fetching the Sudoku Exchange puzzle bank's ${band}.txt")
        file(DOWNLOAD
             "https://raw.githubusercontent.com/grantm/sudoku-exchange-puzzle-bank/${SUDOKU_PUZZLE_BANK_COMMIT}/${band}.txt"
             "${file}.part" EXPECTED_HASH SHA256=${hash} TLS_VERIFY ON STATUS status)
        list(GET status 0 code)
        if(NOT code EQUAL 0)
            file(REMOVE "${file}.part")
            list(GET status 1 reason)
            message(FATAL_ERROR "The Sudoku Exchange puzzle bank's ${band}.txt could not be fetched (${reason}). Build "
                                "without it, the app offering no new puzzles, with -DSUDOKU_PUZZLE_BANK=OFF.")
        endif()
        file(RENAME "${file}.part" "${file}")
    endif()
    list(APPEND SUDOKU_PUZZLE_BANK_FILES "${file}")
endforeach()
