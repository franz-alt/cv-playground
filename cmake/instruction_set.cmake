# set compile options of a target depending on the "USE_INSTRUCTION_SET" variable
function(cvpg_target_instruction_set_compile_options _target)
    if(TARGET ${_target})
        if(USE_INSTRUCTION_SET STREQUAL "AVX2")
            target_compile_options(${_target} PRIVATE $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>:-mavx2>)
            target_compile_options(${_target} PRIVATE $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>:-mfma>)
        endif()
    endif()
endfunction()
