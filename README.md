# P3
CSCI 4061 - Fall 2024 - Project #4
<br/>

# Project group number
Group 5
# Group member names and x500s
Luke Lopata (lopat018), Dan Hawker (hawke069), Samuel Thorson (thors648)
# The name of the CSELabs computer that you tested your code on
login01.cselabs.umn.edu
# Contributions
Everyone worked together to finish this project. Documentation, coding, and testing were all done on call together.
# Any changes you made to the Makefile or existing files that would affect grading
Added output directory creation and server log cleaning to make/ make clean.
# How could we make this better?
To make this better, we could enable each individual request to run in parallel. This would probably be done in the image_match() function. Instead of using one thread to progress through the image, we could have multiple threads progress through the image at the same time. Each thread could be responsible for a row, and at the end all of the results could be used to find the "closest match". Although the pseeudocode below uses dynamic thread creation, it might be even better to have a thread pool. Pseudocode:
```
function handle_row(row):
    for i = 1 to row.length do
        //do some comparison
    return val

function image_match (image):
    for i = 1 to rows do
        pthread_create(handle_row)
    for i = 1 to rows do
        pthread_join
    return matched_image

```