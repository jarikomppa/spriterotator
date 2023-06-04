# spriterotator
Non-destructive sprite rotator

Rotates sprites in a non-destructive manner using the three shears method:
https://cohost.org/tomforsyth/post/891823-rotation-with-three

Since the rotation is performed with point sampled shears, every frame
of the animation contains all the original pixels, just in a different
configuration.

The tool takes in image files (most common formats supported) and the number
of desired output frames.

The tool outputs separate .png frames as well as a .gif animation preview.

