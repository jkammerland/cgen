function(get_cpm version hash_sum)
  if(${CPM_INITIALIZED})
    message(DEBUG "CPM is already initialized in this file: ${CMAKE_CURRENT_SOURCE_DIR}")
  else()
    message(STATUS "Installing CPM ${version}, expecting hash <${hash_sum}>")
    
    set(CPM_DOWNLOAD_VERSION ${version})
    set(CPM_HASH_SUM ${hash_sum})

    if(DEFINED ${CPM_SOURCE_CACHE})
      message(STATUS "CPM_SOURCE_CACHE: ${CPM_SOURCE_CACHE}")
    else()
      message(STATUS "CPM_SOURCE_CACHE is not defined, using default location: ${CMAKE_BINARY_DIR}/cmake")
    endif()
    if(CPM_SOURCE_CACHE)
      if(CPM_SOURCE_CACHE STREQUAL "")
        set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
      else()
        set(CPM_DOWNLOAD_LOCATION "${CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
      endif()
    elseif(DEFINED ENV{CPM_SOURCE_CACHE})
      set(CPM_DOWNLOAD_LOCATION "$ENV{CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
    else()
      set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
    endif()

    # Expand relative path. This is important if the provided path contains a tilde (~)
    get_filename_component(CPM_DOWNLOAD_LOCATION ${CPM_DOWNLOAD_LOCATION} ABSOLUTE)

    file(DOWNLOAD
        https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake
        ${CPM_DOWNLOAD_LOCATION} EXPECTED_HASH SHA256=${CPM_HASH_SUM}
    )

    message(DEBUG "using CPM at ${CPM_DOWNLOAD_LOCATION}")
    include(${CPM_DOWNLOAD_LOCATION})
  endif()
endfunction()