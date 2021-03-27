# FTP-Mohammad
## FTP server and client

This project uses 2 main files 

- FTPclient.c
- FTPserver.c

and accompanying files

### Command List

#### USER <username>
logs into an existing user
  
Tests:
- checked unavailable user: got expected result
- checked a user in NEWU creation: got expected result
- checked available user: got expected result

#### PASS <password>
authenticates a logged in user or creates a password for a NEWU (blank password is allowed)
  
Tests:
- checked wrong password: got expected result
- checked new password: got expected result
- checked correct password: got expected result
- checked using "NULL" as a pass: got expected result

#### NEWU <username>
creates a new user
  
Tests:
- checked user that already exists: got expected result
- checked no username: got expected result
- checked valid username: got expected result

#### LS
lists all files and folders in the current server directory

Tests:
- used in the folder the executable is in: result does not come out similar to !LS (vertically listed and no folder details)
- used in folders other than the initial: got same result as the above

#### !LS
lists all files and folders in the current client directory

Tests:
- used in the folder the executable is in: got expected result
- used in folders other than the initial: got expected result

#### PWD
shows path of the current directory in the server side

Tests:
- used in the folder the executable is in: got expected result
- used in folders other than the initial: got expected result

#### !PWD
shows path of the current directory in the client side

Tests:
- used in the folder the executable is in: got expected result
- used in folders other than the initial: got expected result

#### CD <newDirectory>
changes the current directory to the newDirectory if possible server side

Tests:
- tested directory in current location: got expected result
- tested directory that is invalid: got expected result
- tested CD ..: got expected result

#### !CD <newDirectory>
changes the current directory to the newDirectory if possible client side

Tests:
- tested directory in current location: got expected result
- tested directory that is invalid: got expected result
- tested CD ..: got expected result

#### PUT <fileName>
copies a file from client to server current path (rewrites existing one if new one has the same name as another in the directory), the fileName must include any visible extensions like .c etc.
  
Tests:
- tested existing file in client but not in server: got expected result
- tested existing file in both: the file on the server side was rewritten
- tested on invalid fileName: got expected result
- spam test: got Bind ERROR sometimes, tried to resolve bind using a global (then main()) variable, but it kept resetting for some reason

#### GET <fileName>
copies a file from client to server current path (rewrites existing one if new one has the same name as another in the directory), the fileName must include any visible extensions like .c etc.
  
Tests:
- tested existing file in server but not in client: got expected result
- tested existing file in both: the file on the client side was rewritten
- tested on invalid fileName: got expected result
- spam test: got Bind ERROR sometimes, tried to resolve bind using a global (then main()) variable, but it kept resetting for some reason

#### QUIT
exits the client side

to exit server side, user CTRL + C
