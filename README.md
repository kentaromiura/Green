Green
=====

`Green` is a new way to code _well, not really_.
 
 Just run `Green` on the root of your (currently only git) repository and, as long as it has a `test` file (or `test.bat` for Windows), it will automatically commit each of your changes once tests passes.


When you're done with your changes you can just squash all the commits, and give them the most appropriate message to be sure that your code will always work as _you_ intended.


Green needs `watchman` installed in order to work.
https://facebook.github.io/watchman/


Third party code and libraries and their license:
---    
- `nlohmann/json`   MIT
- `libgit2`         GPL v2 with link exception
- `libgit2pp`       Apache-2.0 https://github.com/marcelocantos/libgit2pp/blob/master/LICENSE


With `Green` you can write code with the peace of mind that what matter is saved and you can go back to it at any time (well, as long as it passes the tests).
