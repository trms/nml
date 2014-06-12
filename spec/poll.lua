local llthreads = require'llthreads'
local nml=require'nml'

local AF_SP = nml.sym.AF_SP.value
local NN_PAIR = nml.sym.NN_PAIR.value
local NN_DONTWAIT = nml.sym.NN_DONTWAIT.value
local NN_POLLIN = 1 -- nml.sym.NN_POLLIN.value
local NN_POLLOUT = 2 -- nml.sym.NN_POLLOUT.value
local SOCKET_ADDRESS = "inproc://a"
local NN_RCVFD = nml.sym.NN_RCVFD.value
local NN_SNDFD = nml.sym.NN_SNDFD.value

local routine1 = [[
{
   nn_sleep (10);
   test_send (sc, "ABC");
}]]

local routine2 = [[
{
   nn_sleep (10);
   nn_term ();
}]]

local NN_IN = 1
local NN_OUT = 2

local getevents = function(s, events, timeout)
    local rc;
    local pollset = {}
    local rcvfd;
    local sndfd;
	local tv = {}
	local revents;

    if events & NN_IN == NN_IN then
        rc, rcvfd = nml.getsockopt(s, NN_SOL_SOCKET, NN_RCVFD)
        errno_assert (rc == 0);
        nn_assert (fdsz == sizeof (rcvfd));
        FD_SET (rcvfd, &pollset);
	end 

    if (events & NN_OUT) {
        fdsz = sizeof (sndfd);
        rc = nn_getsockopt (s, NN_SOL_SOCKET, NN_SNDFD, (char*) &sndfd, &fdsz);
        errno_assert (rc == 0);
        nn_assert (fdsz == sizeof (sndfd));
        FD_SET (sndfd, &pollset);
#if !defined NN_HAVE_WINDOWS
        if (sndfd + 1 > maxfd)
            maxfd = sndfd + 1;
#endif
    }

    if (timeout >= 0) {
        tv.tv_sec = timeout / 1000;
        tv.tv_usec = (timeout % 1000) * 1000;
    }
#if defined NN_HAVE_WINDOWS
    rc = select (0, &pollset, NULL, NULL, timeout < 0 ? NULL : &tv);
    wsa_assert (rc != SOCKET_ERROR);
#else
    rc = select (maxfd, &pollset, NULL, NULL, timeout < 0 ? NULL : &tv);
    errno_assert (rc >= 0);
#endif
    revents = 0;
    if ((events & NN_IN) && FD_ISSET (rcvfd, &pollset))
        revents |= NN_IN;
    if ((events & NN_OUT) && FD_ISSET (sndfd, &pollset))
        revents |= NN_OUT;
    return revents;
}
]]
local rc
local sb
local sc
local buf = {}

--struct nn_thread thread;
local pfd = {}

-- Test nn_poll() function
sb = nml.socket(AF_SP, NN_PAIR)
nml.bind(sb, SOCKET_ADDRESS)

sc = nml.socket(AF_SP, NN_PAIR)
nml.connect(sc, SOCKET_ADDRESS)
nml.send(sc, "ABC", NN_DONTWAIT)
nml.sleep(100)

pfd[1] = {fd = sb, events = NN_POLLIN|NN_POLLOUT}
pfd[2] = {fd = sc, events = NN_POLLIN|NN_POLLOUT}
rc = nml.poll(pfd, 2, -1)
assert(nml.errno()>=0)
assert(rc == 2)
assert(pfd[1].revents == NN_POLLIN|NN_POLLOUT)
assert(pfd[2].revents == NN_POLLOUT)
nml.close(sc)
nml.close(sb)

-- /*  Create a simple topology. */
sb = nml.socket(AF_SP, NN_PAIR)
nml.bind(sb, SOCKET_ADDRESS)
sc = nml.socket(AF_SP, NN_PAIR)
nml.connect(sc, SOCKET_ADDRESS)

-- /*  Check the initial state of the socket. */
rc = getevents (sb, NN_IN | NN_OUT, 1000);
nn_assert (rc == NN_OUT);

/*  Poll for IN when there's no message available. The call should
    time out. */
rc = getevents (sb, NN_IN, 10);
nn_assert (rc == 0);

/*  Send a message and start polling. This time IN event should be
    signaled. */
test_send (sc, "ABC");
rc = getevents (sb, NN_IN, 1000);
nn_assert (rc == NN_IN);

/*  Receive the message and make sure that IN is no longer signaled. */
test_recv (sb, "ABC");
rc = getevents (sb, NN_IN, 10);
nn_assert (rc == 0);

/*  Check signalling from a different thread. */
nn_thread_init (&thread, routine1, NULL);
rc = getevents (sb, NN_IN, 1000);
nn_assert (rc == NN_IN);
test_recv (sb, "ABC");
nn_thread_term (&thread);

/*  Check terminating the library from a different thread. */
nn_thread_init (&thread, routine2, NULL);
rc = getevents (sb, NN_IN, 1000);
nn_assert (rc == NN_IN);
rc = nn_recv (sb, buf, sizeof (buf), 0);
nn_assert (rc < 0 && nn_errno () == ETERM);
nn_thread_term (&thread);

/*  Clean up. */
test_close (sc);
test_close (sb);
]]