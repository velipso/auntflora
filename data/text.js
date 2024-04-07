const text = `
Aunt Flora's Mansion
by anna anthropy
Game Boy Advance port by velipso (code, music) and CobraLaserFace (HD graphics) PocketPulp www.pulp.biz
0123456789
Start game
D-pad to move
A to action
B to undo
Select to restart
Start to change display
A to continue

A letter from Great-Aunt Flora!
She wants me to join her for tea.

Auntie Flora's mansion is full of
so much junk, though! How does she
manage it all at her age?

Step into a Heart and press A to
save your game! Press Select to return
to your last save! It's possible,
but hopefully not easy, to get
stuck - be careful saving!

"Oh hello, Sweetheart. Won't you
join your Auntie for a cup of
tea?"

"Auntie, I don't know how you can
live in this huge place all by
yourself."

"I'm not alone, I have Catsup
here. Now drink your tea before it
gets cold."

"Okay, Auntie. I missed you."

created by anna anthropy

with help from Alan Hazelden,
Jonah Ostroff and Jamie Perconti

playtested by Jen Ada, John H.,
Chris Harris, and Kelsey Higham

The Main Hall
The Kitchen
The Back Stairway
The Back Porch
The Side Gate
The Storage Room
The Colonnade Ballroom
The Annex
The Cellar
The Parlor
The Terrace
The Study
The Library
The Secret Passage
The Wine Cellar
The Conservatory
The Attic
The Dining Room
`;

const chars = Array.from(new Set(text.replace(/\n/g, '').split('')));
chars.sort((a, b) => a.charCodeAt(0) - b.charCodeAt(0));
console.log(chars.length, chars);
