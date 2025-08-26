# Image Steganography

Steganography is the process of hiding a secret message within a larger one in such a way that someone can not know the presence or contents of the hidden message. The purpose of Steganography is to maintain secret communication between two parties. Unlike cryptography, which conceals the contents of a secret message, steganography conceals the very fact that a message is communicated. <br>
For more Details visit - [What is steganography? A complete guide to the ancient art of concealing messages](https://portswigger.net/daily-swig/what-is-steganography-a-complete-guide-to-the-ancient-art-of-concealing-messages)


### Types of Steganography

Steganography is of many types as given in the picture 
![0_0PvWnJdRtDMkh8JS](https://user-images.githubusercontent.com/50414959/126046108-57b6a74f-22e6-451e-89c4-0c6c9e4bfdd2.png)


### Process being employed

The method/architecture being employed
![Steganography-Process](https://user-images.githubusercontent.com/50414959/126046311-723b81bb-bdf0-41b1-a172-6b14f247ae90.png)


### Code Explanation
The code is writtern in as a command line argument format with the appropriate commands as shown in the snippets provided below.<br>
Here we are using the [Least Significant Bit](https://ieeexplore.ieee.org/document/6949808) Steganography technique of performing the encoding and decoding.


#### Text_Encoder.py
![image](https://user-images.githubusercontent.com/50414959/126045952-845d8da0-f437-4f6d-a748-1bda174ee82f.png)

#### Steps done in the code
1. We first read the image as well as the text file which contains the message to be encoded and send it into a function that does the encoding.
2. We then extract the message from the text file and then check if the size of the text to be encoded is less than the size of the image, else an error message is raised
3. The delimiter string is added and then the entire message is sent to a funtion that converts the entire message into a binary value.
4. We then substitute each bit, after converting the pixel values into their respective binary values, into the Least Significant bit of each pixel until the all the bits of the message are substituted.
5. We then return the image and save it with the name of the filename along with an "\_encoded.png" 


#### Text_Decoder.py
![image](https://user-images.githubusercontent.com/50414959/126045942-b59f62aa-0a53-4a86-9ab7-1a57e0a1e31b.png)

#### Steps done in the code
1. We read the encoded image and send it into the function that decodes the image.
2. The image is then passed into a function that extracts the bits from the LSB of each of the pixels of the image.
3. We then group the extracted bits into groups containing 8 bits each, and then pass it into the function that converts the bytes data into characters, which are then grouped to form our actual encoded message.
4. We then return the message obtained and then print it into a text file named "Extracted_msg.txt".

### Sample Inputs and Outputs

#### Sample Images and text used
![test_1_306x450](https://user-images.githubusercontent.com/50414959/126046697-8b1bb468-c1ac-45a9-bb76-5534307fe30f.png)

_Hey, This is Sai Tharun here and the person who wrote this cool bit of code. This text is the secret message that is to be hidden in the picture._


#### Sample encoded image obtained
![test_encoded_1_306x450](https://user-images.githubusercontent.com/50414959/126046721-495ef9fc-676e-42ba-9efe-bb3152e3742b.png)

As you can see there is no difference between the Sample Image and the encoded image, yet the text is encoded into the image. 

Seems Magical isn't it!
