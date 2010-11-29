#/bin/sh
echo "Now, you can read printk DEBUG message..."
echo > /proc/sys/kernel/printk "8"
