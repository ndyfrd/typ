# typ
Small terminal program for Linux that emulates a typewriter. This is strict emulation so no backspace. Pages are limited to 60 characters by 30 lines (standard typewritten page from the olden days). This program was created to help writers forget about editing and just write. No word counts. No backspace. No auto-wrapping. No training wheels.

# Usage

1. Create a directory where your document pages will be kept.
2. Run typ in this direcory. typ will create a new page file called 'page_1'. Begin typ-ing...
3. Once you have reached the end of the page press Shift-n to create and load a new page. Typ will load 'page_2'. Continue typ-ing...
4. To exit press Esc. typ will save your page on exit. 
5. To start from where you left off just run typ from the directory. typ will load the most recent page. 
