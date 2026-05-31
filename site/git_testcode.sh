./busybox echo "#### OS COMP TEST GROUP START git ####"
./busybox echo "here1"
./usr/bin/git config --global --add safe.directory "$(pwd)"
./busybox echo "here2"
./usr/bin/git config --global user.email "you@example.com"
./busybox echo "here3"
./usr/bin/git config --global user.name "Your Name"
./busybox echo "here4"
./usr/bin/git help
./busybox echo "here5"
./usr/bin/git init
./busybox echo "here6"
./busybox echo "hello world" > README.md
./busybox echo "here7"
./usr/bin/git add README.md
./busybox echo "here8"
./usr/bin/git commit -m "add README.md"
./busybox echo "here9"
./usr/bin/git log
./busybox echo "here10"
./busybox echo "#### OS COMP TEST GROUP END git ####"
