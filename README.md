# spriterotator
Non-destructive sprite rotator

![Animation](https://raw.github.com/jarikomppa/spriterotator/master/smallsprite.png_anim.gif)

Rotates sprites in a non-destructive manner using the three shears method:
https://cohost.org/tomforsyth/post/891823-rotation-with-three

Since the rotation is performed with point sampled shears, every frame
of the animation contains all the original pixels, just in a different
configuration.

The tool takes in image files (most common formats supported) and the number
of desired output frames.

The tool outputs separate .png frames as well as a .gif animation preview.

Options:
 -f output frames
 -s output safe frame
 -g output preview gif animation
 -i output intermediary files
 
 Frames are what you most likely want eventually.
 
 Safe frame shows which pixels are retained when rotating. The safe frame depends on
 the resolution and number of desired steps.

![Safe frame](https://raw.github.com/jarikomppa/spriterotator/master/smallsprite.png_safe.png)

Gif animation is handy to see how the rotation works for your input file without having to parse lots of single frame images.

Intermediary files show how the shears look like, and may be useful for debugging.

