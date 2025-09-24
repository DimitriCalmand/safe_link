# Safe_Link

Safe_Link is a student project for spam detection in GMAIL emails, written in C. It uses the Google API to interact with your GMAIL account and scans the links in emails to identify and filter out spam.

## Features

*   Real-time analysis of incoming emails in your GMAIL inbox.
*   Detection of potential spam or phishing links within emails.
*   Automatic categorization and filtration of emails identified as spam.
*   Safe interaction with GMAIL through the use of Google's API.

## Installation 

Here are the instructions to set up Safe_Link on your system:

1. Clone the repository
First, clone the Safe_Link repository to your local system:
```bash
git clone git@gitlab.com:dipezed/safe_link.git
```
2. Build the project
Navigate to the project directory and use the 'make' command to build the project:
```bash
cd safe_link
make
```
3. Run Safe_Link
Once you've build our project you can export our dependency and run Safe_Link:
```bash
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./lib
./safe_link
```
