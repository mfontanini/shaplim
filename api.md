# shaplim API

The API uses JSON encoded object delimited by a newline character. 

Therefore, every command should look like this:

`{ ... }\n`

All of the commands *must* contain a `type` key, which will be the
command identifier. Depending on the command type, a `params` key might
be expected, which should contain the needed parameter(s) for it.

The type of the object stored inside the `params` key depends on the
type of the command. 

## List shared directories

This command lists all of the shared directories in the server. Remote
clients can only play music that is stored inside one of them.

* Command type: `list_shared_dirs`
* Example: 
```javascript
{ 
    "type" : "list_shared_dirs" 
}```
* Output: 
```javascript
{ 
    "result" : bool, 
    "directories" : [ "dir1", "dir2", ... ]
}
```

## List subdirectories/files inside a shared directory

This command lists files and directories inside any shared directory or
its subdirectories.

* Command type: `list_directory`
* Takes a `string` as parameter.
* Example: 
```javascript
{ 
    "type" : "list_directory", 
    "params" : "dir1" 
}```
* Output: 
```javascript
{ 
    "result" : bool, 
    "directories" : [ 
        "dir1", 
        "dir2", 
        ... 
    ], 
    "files" : [ 
        "file1", 
        "file2", 
        ... 
    ]
}
```

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
* Example: 
```javascript
{ 
    "type" : "add_shared_songs", 
    "params" : { 
        "base_path" : "music/tests", 
        "songs" : [ 
            "a.mp3", 
            "b.mp3" 
        ] 
    } 
}
```
* Output: 
```javascript
{ 
    "result" : bool 
}```


## Play

* Command type: `play`
* Example:
```javascript
{
    "type" : "play"
}
```
* Output: 
```javascript
{ 
    "result" : bool
}
```

## Pause

* Command type: `pause`
* Example:
```javascript
{
    "type" : "pause"
}
```
* Output: 
```javascript
{ 
    "result" : bool
}
```

## Next song

* Command type: `next_song`
* Example:
```javascript
{
    "type" : "next_song"
}
```
* Output: 
```javascript
{ 
    "result" : bool
}
```

## Previous song

* Command type: `previous_song`
* Example:
```javascript
{
    "type" : "previous_song"
}
```
* Output: 
```javascript
{ 
    "result" : bool
}
```

## Playlist mode

This command retrieves the playlist mode, which can be either `default`
or `shuffle`.

* Command type: `playlist_mode`
* Example:
```javascript
{
    "type" : "playlist_mode"
}
```
* Output: 
```javascript
{ 
    "result" : bool, 
    "mode" : string
}
```

## Set playlist mode

This command retrieves sets the playlist mode. 

* Command type: `playlist_mode`
* Takes a `string` as a parameter, which *must* be either `default` or 
`shuffle`.
* Example:
```javascript
{
    "type" : "playlist_mode"
}
```
* Output: 
```javascript
{ 
    "result" : bool, 
    "mode" : string
}
```
## Clear playlist

Removes all of the songs from the playlist and stops playing the current
one

* Command type: `clear_playlist`
* Example:
```javascript
{
    "type" : "clear_playlist"
}
```
* Output: 
```javascript
{ 
    "result" : bool
}
```
