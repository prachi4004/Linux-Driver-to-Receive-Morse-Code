#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>
#include <linux/timer.h>
#include <linux/workqueue.h>
#include <linux/slab.h>
#include "morse_driver.h"  // Your IOCTL definitions (make sure this exists!)

static int morse_major;
static struct timer_list morse_timer;
static struct work_struct morse_work;
static char *morse_input_buffer = NULL;
static DEFINE_MUTEX(morse_mutex);
static bool decoding_done = false;  // Flag to track if decoding is done

// Morse code mapping (dot = '.', dash = '-', space = ' ')
const char *morse_code[] = {
    ".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", ".---",
    "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.", "...", "-",
    "..-", "...-", ".--", "-..-", "-.--", "--.."
};

// Morse code decoding function
static char decode_morse_char(const char *morse) {
    int i;
    for (i = 0; i < 26; i++) {
        if (strcmp(morse, morse_code[i]) == 0)
            return 'A' + i;
    }
    return '?'; // Unknown Morse code
}

// Write the decoded Morse character to a file
static void write_to_file(const char *decoded_chars) {
    struct file *f = NULL;
    loff_t pos = 0;
    ssize_t ret;

    f = filp_open("/home/prachi/Downloads/os_project_copy/morse_driver.txt", O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (IS_ERR(f)) {
        printk(KERN_ERR "Failed to open file\n");
        return;
    }

    pos = f->f_pos;
    ret = kernel_write(f, decoded_chars, strlen(decoded_chars), &pos);
    if (ret < 0) {
        printk(KERN_ERR "Failed to write to file\n");
    } else {
        f->f_pos = pos;
    }

    filp_close(f, NULL);
}

// Actual decoding and writing work
static void morse_work_handler(struct work_struct *work) {
    char decoded[256] = {0};
    char morse_letter[16];
    int i = 0, j = 0, k = 0;

    if (!morse_input_buffer)
        return;

    mutex_lock(&morse_mutex);

    if (decoding_done) {
        printk(KERN_INFO "Morse decoding already done, skipping.\n");
        mutex_unlock(&morse_mutex);
        return;
    }

    while (morse_input_buffer[i] != '\0') {
        if (morse_input_buffer[i] != ' ') {
            morse_letter[j++] = morse_input_buffer[i];
        } else {
            morse_letter[j] = '\0';
            decoded[k++] = decode_morse_char(morse_letter);
            j = 0;
        }
        i++;
    }

    // Handle last Morse character if no trailing space
    if (j > 0) {
        morse_letter[j] = '\0';
        decoded[k++] = decode_morse_char(morse_letter);
    }
    decoded[k++] = '\n';  // Newline after each decode session
    decoded[k] = '\0';

    write_to_file(decoded);
    decoding_done = true;  // Mark decoding as done

    mutex_unlock(&morse_mutex);
}

// Timer callback
static void morse_timer_callback(struct timer_list *t) {
    schedule_work(&morse_work);  // Safe: schedule work in process context
    mod_timer(&morse_timer, jiffies + msecs_to_jiffies(200));  // Restart timer
}

// IOCTL handler
static long morse_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    char user_buf[256];

    switch (cmd) {
        case MORSE_IOC_SEND:
            if (copy_from_user(user_buf, (char __user *)arg, sizeof(user_buf) - 1))
                return -EFAULT;
            user_buf[sizeof(user_buf) - 1] = '\0';

            printk(KERN_INFO "Morse code received: %s\n", user_buf);

            // Decode the Morse code and write to file
            mutex_lock(&morse_mutex);
            kfree(morse_input_buffer);
            morse_input_buffer = kstrdup(user_buf, GFP_KERNEL);
            decoding_done = false;  // Reset the flag when new input is received
            mutex_unlock(&morse_mutex);

            // Process and write the decoded morse code
            schedule_work(&morse_work);
            break;

        default:
            return -EINVAL;
    }

    return 0;
}


// File operations
static const struct file_operations morse_fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = morse_ioctl,
};

// Module initialization
static int __init morse_driver_init(void) {
    morse_major = register_chrdev(0, "morse_driver", &morse_fops);
    if (morse_major < 0) {
        printk(KERN_ALERT "Morse driver failed to register\n");
        return morse_major;
    }

    INIT_WORK(&morse_work, morse_work_handler);

    timer_setup(&morse_timer, morse_timer_callback, 0);
    mod_timer(&morse_timer, jiffies + msecs_to_jiffies(200));

    printk(KERN_INFO "Morse driver registered with major number %d\n", morse_major);
    return 0;
}

// Module cleanup
static void __exit morse_driver_exit(void) {
    del_timer_sync(&morse_timer);
    cancel_work_sync(&morse_work);

    mutex_lock(&morse_mutex);
    kfree(morse_input_buffer);
    mutex_unlock(&morse_mutex);

    unregister_chrdev(morse_major, "morse_driver");
    printk(KERN_INFO "Morse driver unregistered\n");
}

module_init(morse_driver_init);
module_exit(morse_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Prachi Raj");
MODULE_DESCRIPTION("Morse code input driver (corrected version)");
MODULE_VERSION("1.0");
