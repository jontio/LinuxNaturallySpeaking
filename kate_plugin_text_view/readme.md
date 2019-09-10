For me...

```bash
mkdir build
cd build
cmake ..
make
sudo mv ktexteditor_textview.so /usr/lib/x86_64-linux-gnu/qt5/plugins/ktexteditor/ktexteditor_textview.so 
```

After you've installed the plugin "ktexteditor_textview.so" you will need to enable the plug in indicate by going to "settings->configure Kate->plugins" and ticking the "TextView" plugin.

For this plug-in I copied a how to example that was designed for an older version of Kate. To get it working with the new version of Kate I modified a CMakeLists.txt file that came with the Kate source. There's definitely things that aren't needed in the CMakeLists.txt file but I've left them there.


