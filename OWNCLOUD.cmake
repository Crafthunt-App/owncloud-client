set( APPLICATION_NAME       "BauGPT" )
set( APPLICATION_SHORTNAME  "BauGPT" )
set( APPLICATION_EXECUTABLE "baugpt" )
set( APPLICATION_DOMAIN     "baugpt.pro" )
set( APPLICATION_VENDOR     "BauGPT" )
set( APPLICATION_UPDATE_URL "" CACHE STRING "URL for updater" )  # Disabled until BauGPT infrastructure ready
set( APPLICATION_ICON_NAME  "owncloud" )  # Kept as 'owncloud' for backwards compatibility with icon filenames
set( APPLICATION_VIRTUALFILE_SUFFIX "baugpt" CACHE STRING "Virtual file suffix (not including the .)")

set( LINUX_PACKAGE_SHORTNAME "baugpt" )

set( THEME_CLASS            "ownCloudTheme" )
set( APPLICATION_REV_DOMAIN "com.baugpt.desktopclient" )
set( WIN_SETUP_BITMAP_PATH  "${CMAKE_SOURCE_DIR}/admin/win/nsi" )

set( MAC_INSTALLER_BACKGROUND_FILE "${CMAKE_SOURCE_DIR}/admin/osx/installer-background.png" CACHE STRING "The MacOSX installer background image")

set( THEME_INCLUDE          "owncloudtheme.h" )

# set( THEME_INCLUDE          "${OEM_THEME_DIR}/mytheme.h" )
# set( APPLICATION_LICENSE    "${OEM_THEME_DIR}/license.txt )

option( WITH_CRASHREPORTER "Build crashreporter" OFF )
set( CRASHREPORTER_SUBMIT_URL "" CACHE STRING "URL for crash reporter" )  # Disabled

