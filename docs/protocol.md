
# Key-Value Store v0.1 Protocol

ASCII Commands:
SET <key> <val>
GET <key>
DEL <key>

Output:
$val -> returned value
SUCCESS! -> command succeeded
NULL! -> value not found
VERB COULD NOT BE PARSED! -> failed to parse command
