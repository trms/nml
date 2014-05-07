# WHERE AT


## pub sub

pubsub almost work. There is no clean way to designate the channel that a message should go on. The way it works is:

Sending: "channel\0message..."

If a sub socket has a subscription that matches the "channel" portion, then it receives the whole string.

It would be better if: the pub socket had a seperate field for channel and sub received two return values: msg, channel.

## rep req

nanomsg has a bug in it where it doesn't time out correctly. The code is hacked to make this kinda-work, but it doesn't yet work completely.

## select/epoll/iocp

The file descriptors need to be used to integrate the socket with event systems.

## open source

I'd like to figure out a clean way to keep these projects open sourced, while still having a way to have our projects / solutions. I think that submodules are going to be in our future.
