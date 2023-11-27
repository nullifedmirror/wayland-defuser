# wayland-defuser

Makes wayland go away! (breaking anything wayland related in the process)

This is nothing personal against Wayland, I think it's neat but it suffers from many issues and the slow development process continues to make me more critical of FreeDesktop as a whole.

## WARNING

This **WILL** break **ANYTHING** that uses Wayland exclusively. This includes compositors, Wayland-only terminals and even Mir!

To make breaking things even more fun, I have forced Wayland Protocols to be blanked out, this is intentional to prevent any compilation of Wayland features.
