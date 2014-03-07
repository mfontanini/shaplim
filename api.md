# shaplim API

The API uses JSON encoded string delimited by a newline character. 

Therefore, every command should look like this:

`{ ... }\n`

## List shared directories

This command lists all of the shared directories in the server. Remote
clients can only play music that is stored inside one of them.

* Command type: `list_shared_dirs`
* Example: `{ "type" : "list_shared_dirs" }`
* Output: `{ "result" : bool, "directories" : [ "dir1", "dir2", ... ]}`

## List subdirectories/files inside a shared directory

This command lists files and directories inside any shared directory or
its subdirectories.

* Command type: `list_directory`
* Takes a `string` as parameter.
* Example: `{ "type" : "list_directory", "params" : "dir1" }` 
* Output: `{ "result" : bool, "directories" : [ "dir1", "dir2", ... ], "files" : [ "file1", "file2", ... ]}`

## Add shared songs

This command adds songs that are stored in the shared directories to the
playlist. It takes a base path and the names of all of the song names to
be added.

All of the provided song names *must* be direct children of the base 
path. This means that if you want to add two songs, `a.mp3`, `b.mp3` 
that are located inside the shared directory `music/tests/` then you 
should provide `music/tests` as the base path and `a.mp3`, `b.mp3` as 
the names of the songs to be played.

* Command type: `add_shared_songs`
* Takes an `object` as a parameter, which *must* contain a `base_path`
key which holds a string and a `songs` key which holds a `[string]`.
* Example: `{ "type" : "add_shared_songs", "params" : { "base_path" : "music/tests", "songs" : [ "a.mp3", "b.mp3" ] } }`
* Output: `{ "result" : bool }`
