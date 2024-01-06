unix {
INCLUDEPATH += $$MYPAINT_DIR/json-c $$MYPAINT_DIR/libmypaint $$MYPAINT_DIR/qtmypaint $$MYPAINT_DIR/raster/brushes $$MYPAINT_DIR/raster/color $$MYPAINT_DIR/raster/size $$MYPAINT_DIR/raster/main
LIBS += -L$$MYPAINT_DIR/json-c -L$$MYPAINT_DIR/libmypaint -L$$MYPAINT_DIR/qtmypaint -L$$MYPAINT_DIR/raster/brushes -L$$MYPAINT_DIR/raster/color -L$$MYPAINT_DIR/raster/size -L$$MYPAINT_DIR/raster/main 
LIBS += -ljson-c -llibmypaint -lqtmypaint -lrasterbrushes -lrastercolor -lrastersize -lrastermain
}

win32 {
INCLUDEPATH += $$MYPAINT_DIR/json-c $$MYPAINT_DIR/libmypaint $$MYPAINT_DIR/qtmypaint $$MYPAINT_DIR/raster/brushes $$MYPAINT_DIR/raster/color $$MYPAINT_DIR/raster/size $$MYPAINT_DIR/raster/main
LIBS += -L$$MYPAINT_DIR/json-c/release -L$$MYPAINT_DIR/libmypaint/release -L$$MYPAINT_DIR/qtmypaint/release -L$$MYPAINT_DIR/raster/brushes/release -L$$MYPAINT_DIR/raster/color/release -L$$MYPAINT_DIR/raster/size/release -L$$MYPAINT_DIR/raster/main/release
LIBS += -ljson-c -llibmypaint -lqtmypaint -lrasterbrushes -lrastercolor -lrastersize -lrastermain 
}
