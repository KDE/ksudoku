# SPDX-FileCopyrightText: 2024 Friedrich W. H. Kossebau <kossebau@kde.org>
#
# SPDX-License-Identifier: BSD-3-Clause

function(install_theme name)
    cmake_parse_arguments(ARG "" "SVG;PREVIEW" "" ${ARGN})
    if (NOT ARG_SVG)
        set(ARG_SVG "${name}.svg")
    endif()
    if (NOT ARG_PREVIEW)
        set(ARG_PREVIEW "${name}_preview.png")
    endif()
    set(svgz "${CMAKE_CURRENT_BINARY_DIR}/${ARG_SVG}z")
    generate_svgz(${ARG_SVG} ${svgz} "theme-")
    install(
        FILES
            ${name}.desktop
            ${ARG_PREVIEW}
            ${svgz}
        DESTINATION ${KDE_INSTALL_DATADIR}/ksudoku/themes
    )
endfunction()

install_theme(abstraction)
install_theme(ksudoku_scrible PREVIEW scribble_preview.png)
install_theme(default SVG ksudoku_egyptian.svg PREVIEW egyptian_preview.png)
