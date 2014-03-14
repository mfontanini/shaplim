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
}
```
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
}
```
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
}
```


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
## Delete song

Deletes a specific song in the playlist. A timestamp must be provided to make sure that the song being deleted is actually the desired one.

* Command type: `delete_song`
* Example:
```javascript
{
    "type" : "delete_song",
    "index" : int,
    "timestamp" : int
}
```
* Output: 
```javascript
{ 
    "result" : bool
}
```
## Set current song

Plays a song that is already in the playlist. A timestamp must be provided to make sure that the song being played is actually the desired one.

* Command type: `set_current_song`
* Example:
```javascript
{
    "type" : "set_current_song",
    "index" : int,
    "timestamp" : int
}
```
* Output: 
```javascript
{ 
    "result" : bool
}
```
## Show playlist

Retrieves the current playlist. The current song is indicated as a 0-index based number, whithin the given song list. The timestamp key contains a number that should be used later when using the "new_events" command.

* Command type: `show_playlist`
* Example:
```javascript
{
    "type" : "show_playlist"
}
```
* Output: 
```javascript
{ 
    "result" : bool,
    "songs" : [ string ],
    "current" : int,
    "timestamp" : int
}
```
## New events

Retrieves all of the events that happened from a time point.

* Command type: `new_events`
* Example:
```javascript
{
    "type" : "new_events",
    "timestamp" int
}
```
* Output: 
```javascript
{ 
    "result" : bool,
    "events" : [
        { }
    ],
    "timestamp" : int
}
```
### Events

The events can be have the following structure, depending on the event type:
* Add songs: Indicates that one or more songs were added at the end of the playlist.
```javascript
{
    "type" : "add_songs",
    "songs" : [ string ]
}
```
* Play song: Indicates that some song on the playlist was/is being played. The index is the playlist song number.
```javascript
{
    "type" : "play_song",
    "index" : int
}
```
* Pause: Indicates that the player was paused.
```javascript
{
    "type" : "pause"
}
```
* Play: Indicates that the player changed from state paused to playing.
```javascript
{
    "type" : "play"
}
```
* Playlist mode changed: Indicates that the playlist mode has been changed. The "mode" key can be either "default" or "shuffle".
```javascript
{
    "type" : "playlist_mode_changed",
    "mode" : string
}
```
* Play: Indicates that the player changed from state paused to playing.
```javascript
{
    "type" : "play"
}
```
