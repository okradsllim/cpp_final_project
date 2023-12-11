/*
main.cpp
CSPB 1300 Image Processing Application

PLEASE FILL OUT THIS SECTION PRIOR TO SUBMISSION

- Your name:
    <WILLIAM NYARKO>

- All project requirements fully met? (YES or NO):
    <YES>

- If no, please explain what you could not get to work:
    <ANSWER>

- Did you do any optional enhancements? If so, please explain:
    <Input validation enhancements to prevent input-output file overrides>
    <Preventing program from abruptly ending due to segmentation fault caused by nonexistent file/empty file/.bmp images with just a header, info>
    <Flexibility in file naming>
*/

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <cmath>
#include <limits>
#include <iomanip>
#include <utility>
#include <tuple>
#include <algorithm>

using namespace std;

//***************************************************************************************************//
//                                DO NOT MODIFY THE SECTION BELOW                                    //
//***************************************************************************************************//

// Pixel structure
struct Pixel
{
    // Red, green, blue color values
    int red;
    int green;
    int blue;
};

/**
 * Gets an integer from a binary stream.
 * Helper function for read_image()
 * @param stream the stream
 * @param offset the offset at which to read the integer
 * @param bytes  the number of bytes to read
 * @return the integer starting at the given offset
 */ 
int get_int(fstream& stream, int offset, int bytes)
{
    stream.seekg(offset);
    int result = 0;
    int base = 1;
    for (int i = 0; i < bytes; i++)
    {   
        result = result + stream.get() * base;
        base = base * 256;
    }
    return result;
}

/**
 * Reads the BMP image specified and returns the resulting image as a vector
 * @param filename BMP image filename
 * @return the image as a vector of vector of Pixels
 */
vector<vector<Pixel>> read_image(string filename)
{
    // Open the binary file
    fstream stream;
    stream.open(filename, ios::in | ios::binary);

    // Get the image properties
    int file_size = get_int(stream, 2, 4);
    int start = get_int(stream, 10, 4);
    int width = get_int(stream, 18, 4);
    int height = get_int(stream, 22, 4);
    int bits_per_pixel = get_int(stream, 28, 2);

    // Scan lines must occupy multiples of four bytes
    int scanline_size = width * (bits_per_pixel / 8);
    int padding = 0;
    if (scanline_size % 4 != 0)
    {
        padding = 4 - scanline_size % 4;
    }

    // Return empty vector if this is not a valid image
    if (file_size != start + (scanline_size + padding) * height)
    {
        return {};
    }

    // Create a vector the size of the input image
    vector<vector<Pixel>> image(height, vector<Pixel> (width));

    int pos = start;
    // For each row, starting from the last row to the first
    // Note: BMP files store pixels from bottom to top
    for (int i = height - 1; i >= 0; i--)
    {
        // For each column
        for (int j = 0; j < width; j++)
        {
            // Go to the pixel position
            stream.seekg(pos);

            // Save the pixel values to the image vector
            // Note: BMP files store pixels in blue, green, red order
            image[i][j].blue = stream.get();
            image[i][j].green = stream.get();
            image[i][j].red = stream.get();

            // We are ignoring the alpha channel if there is one

            // Advance the position to the next pixel
            pos = pos + (bits_per_pixel / 8);
        }

        // Skip the padding at the end of each row
        stream.seekg(padding, ios::cur);
        pos = pos + padding;
    }

    // Close the stream and return the image vector
    stream.close();
    return image;
}

/**
 * Sets a value to the char array starting at the offset using the size
 * specified by the bytes.
 * This is a helper function for write_image()
 * @param arr    Array to set values for
 * @param offset Starting index offset
 * @param bytes  Number of bytes to set
 * @param value  Value to set
 * @return nothing
 */
void set_bytes(unsigned char arr[], int offset, int bytes, int value)
{
    for (int i = 0; i < bytes; i++)
    {
        arr[offset+i] = (unsigned char)(value>>(i*8));
    }
}

/**
 * Write the input image to a BMP file name specified
 * @param filename The BMP file name to save the image to
 * @param image    The input image to save
 * @return True if successful and false otherwise
 */
bool write_image(string filename, const vector<vector<Pixel>>& image)
{
    // Get the image width and height in pixels
    int width_pixels = image[0].size();
    int height_pixels = image.size();

    // Calculate the width in bytes incorporating padding (4 byte alignment)
    int width_bytes = width_pixels * 3;
    int padding_bytes = 0;
    padding_bytes = (4 - width_bytes % 4) % 4;
    width_bytes = width_bytes + padding_bytes;

    // Pixel array size in bytes, including padding
    int array_bytes = width_bytes * height_pixels;

    // Open a file stream for writing to a binary file
    fstream stream;
    stream.open(filename, ios::out | ios::binary);

    // If there was a problem opening the file, return false
    if (!stream.is_open())
    {
        return false;
    }

    // Create the BMP and DIB Headers
    const int BMP_HEADER_SIZE = 14;
    const int DIB_HEADER_SIZE = 40;
    unsigned char bmp_header[BMP_HEADER_SIZE] = {0};
    unsigned char dib_header[DIB_HEADER_SIZE] = {0};

    // BMP Header
    set_bytes(bmp_header,  0, 1, 'B');              // ID field
    set_bytes(bmp_header,  1, 1, 'M');              // ID field
    set_bytes(bmp_header,  2, 4, BMP_HEADER_SIZE+DIB_HEADER_SIZE+array_bytes); // Size of BMP file
    set_bytes(bmp_header,  6, 2, 0);                // Reserved
    set_bytes(bmp_header,  8, 2, 0);                // Reserved
    set_bytes(bmp_header, 10, 4, BMP_HEADER_SIZE+DIB_HEADER_SIZE); // Pixel array offset

    // DIB Header
    set_bytes(dib_header,  0, 4, DIB_HEADER_SIZE);  // DIB header size
    set_bytes(dib_header,  4, 4, width_pixels);     // Width of bitmap in pixels
    set_bytes(dib_header,  8, 4, height_pixels);    // Height of bitmap in pixels
    set_bytes(dib_header, 12, 2, 1);                // Number of color planes
    set_bytes(dib_header, 14, 2, 24);               // Number of bits per pixel
    set_bytes(dib_header, 16, 4, 0);                // Compression method (0=BI_RGB)
    set_bytes(dib_header, 20, 4, array_bytes);      // Size of raw bitmap data (including padding)                     
    set_bytes(dib_header, 24, 4, 2835);             // Print resolution of image (2835 pixels/meter)
    set_bytes(dib_header, 28, 4, 2835);             // Print resolution of image (2835 pixels/meter)
    set_bytes(dib_header, 32, 4, 0);                // Number of colors in palette
    set_bytes(dib_header, 36, 4, 0);                // Number of important colors

    // Write the BMP and DIB Headers to the file
    stream.write((char*)bmp_header, sizeof(bmp_header));
    stream.write((char*)dib_header, sizeof(dib_header));

    // Initialize pixel and padding
    unsigned char pixel[3] = {0};
    unsigned char padding[3] = {0};

    // Pixel Array (Left to right, bottom to top, with padding)
    for (int h = height_pixels - 1; h >= 0; h--)
    {
        for (int w = 0; w < width_pixels; w++)
        {
            // Write the pixel (Blue, Green, Red)
            pixel[0] = image[h][w].blue;
            pixel[1] = image[h][w].green;
            pixel[2] = image[h][w].red;
            stream.write((char*)pixel, 3);
        }
        // Write the padding bytes
        stream.write((char *)padding, padding_bytes);
    }

    // Close the stream and return true
    stream.close();
    return true;
}

//***************************************************************************************************//
//                                DO NOT MODIFY THE SECTION ABOVE                                    //
//***************************************************************************************************//

//                                      HELPER FUNCTIONS                                            //
/**
    Checks for input errors and clears the buffer if any are found.

    @returns true if a recent input error occurred and was addressed, prompting the user to retry.
    @returns false if no input errors were detected.
*/  
bool handle_input_error() 
{
    if (cin.fail()) 
    {
        cin.clear(); 
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); //gets rid of max number of 
        cout << "Invalid input. Please try again.\n";
        return true;  // Indicates an error occurred and was handled
    }
    return false;     // Indicates no error
}


/**
    Retrieves the filename from the user, validates the input, 
    ensures the filename has a .bmp extension, and allows exiting the program.

    @param prompt: String message displayed to the user.
    @returns Either the filename or "q", which maps onto "return 0" in main.
*/
string get_filename(const string& prompt) 
{
    string filename;
    while (true) 
    {
        cout << prompt;
        getline(cin, filename);
        
        // If there's an input error, handle and loop again
        if (handle_input_error()) 
        {
            continue;
        }

        // If user decides to quit
        if (filename == "q" || filename == "Q")
        {
            cout << "Exiting program...Goodbye!" << endl;
            return "q";
        }

        // Ensure filename ends with .bmp
        if (!filename.empty() && (filename.length() < 4 || filename.substr(filename.length() - 4) != ".bmp"))
        {
            filename += ".bmp";
        }
        
        return filename;
    }
}


/**
    Ensures the input and output filenames are unique to avoid overwriting.
    
    @param input_filename: Name of the input file.
    @param output_filename: Name of the output file.
    @returns true if filenames are unique, false if they are the same.
*/
bool ensure_unique_output_filename(const string& input_filename, const string& output_filename) 
{
    if (input_filename == output_filename) 
    {
        cout << "Error: Input and Output filenames are the same. Please provide a different output filename." << endl;
        return false; // Names are the same
    }
    return true; // Names are unique
}


/**
    Displays an image editing menu and prompts the user to select an option.
    
    @param filename: The current input file name, displayed in option '0'.
    @returns An integer corresponding to the chosen option for use in a switch statement.
            -2 indicates the program should exit.
*/
int prompt_and_get_menu_choice(const string& filename) 
{
    cout << "\nPlease enter option number or 'q' to quit: " << endl;
    cout << endl;

    cout << setw(32) << left << "(1) Vignette"                   << "(6) Enlarge" << endl;
    cout << setw(32) << left << "(2) Clarendon"                  << "(7) High contrast" << endl;
    cout << setw(32) << left << "(3) Grayscale"                  << "(8) Lighten" << endl;
    cout << setw(32) << left << "(4) Rotate 90 degrees"          << "(9) Darken" << endl;
    cout << setw(32) << left << "(5) Rotate by multiples of 90"  << "(10) Black, white, red, green, blue" << endl;

    cout << "\n0) Change image (current: " << filename << ")" << endl;

    string choice_str;
    int choice;

    while (true) 
    {
        getline(cin, choice_str);

        // Check for input errors
        if (handle_input_error()) 
        {
            continue;
        }

        // Handle program exit
        if (choice_str == "q" || choice_str == "Q") 
        {
            cout << "Exiting program...Goodbye!" << endl;
            return -2;
        }

        // Convert string choice to int and check its range
        stringstream str_to_int_converter(choice_str);
        if (str_to_int_converter >> choice && (choice >= 0 && choice <= 10)) 
        {
            return choice;
        } 
        else 
        {
            cout << "Invalid choice. Please try again: ";
        }
    }
}


/**
    Outputs a success message describing the image editing effect applied based on user's menu choice.
    
    @param choice: User's menu selection.
    No return value; only outputs a success message.
*/
void display_success_message(int choice) 
{
    string editing_effect[] = {
        "",
        "Vignette effect",
        "Clarendon effect",
        "Greyscale effect",
        "Rotate 90 degrees effect",
        "Rotate by multiples of 90 effect",
        "Enlarge effect",
        "High contrast effect",
        "Lightening effect",
        "Darkening effect",
        "Black, white, red, green, blue only effect"
    };

    cout << editing_effect[choice] << " successfully applied!\nCheck directory for your edited photo.\n";
}


/**
    Determines the dimensions of a given image.
    
    @param image: 2D vector representing the image.
    @returns Pair where the first element is the number of rows, and the second is the number of columns.
*/
pair<int, int> get_image_dimensions(const vector<vector<Pixel>>& image) 
{
    int num_rows = image.size();
    int num_columns = image[0].size();
    return make_pair(num_rows, num_columns);
}


/**
    Creates a blank image with given dimensions.
    
    @param num_rows: Desired number of rows for the image.
    @param num_columns: Desired number of columns for the image.
    @returns 2D vector of Pixel representing the blank image.
*/
vector<vector<Pixel>> initialize_new_image(int num_rows, int num_columns) 
{
    return vector<vector<Pixel>>(num_rows, vector<Pixel>(num_columns));
}


/**
    Retrieves the RGB values of a pixel at the specified location in the image.
    
    @param image: 2D vector representing the image.
    @param row: Row index of the desired pixel.
    @param col: Column index of the desired pixel.
    @returns Tuple containing the red, green, and blue values of the pixel.
*/
tuple<int, int, int> get_pixel_rgb(const vector<vector<Pixel>>& image, int row, int col) 
{
    Pixel p = image[row][col];
    return make_tuple(p.red, p.green, p.blue);
}


/**
    Inserts a pixel with given RGB values into the specified location of the new image.
    
    @param new_image: 2D vector representing the target image.
    @param row: Row index for the pixel insertion.
    @param col: Column index for the pixel insertion.
    @param new_red: Red value for the new pixel.
    @param new_green: Green value for the new pixel.
    @param new_blue: Blue value for the new pixel.
*/
void store_pixel(vector<vector<Pixel>>& new_image, int row, int col, int new_red, int new_green, int new_blue) 
{
    Pixel new_pixel = {new_red, new_green, new_blue};
    new_image[row][col] = new_pixel;
}


/**
    Calculates the grayscale value of an RGB pixel using the average of its components.
    
    @param red_value: Red component of the pixel.
    @param green_value: Green component of the pixel.
    @param blue_value: Blue component of the pixel.
    @return The grayscale value.
*/
double grey_value(int red_value, int green_value, int blue_value) 
{
    return (red_value + green_value + blue_value) / 3.0;
}


// (For process1)
/**
    Calculates the scaling factor for the vignette effect based on pixel's distance to the image center.
    
    @param row: Row index of the pixel.
    @param col: Column index of the pixel.
    @param num_rows: Height of the image.
    @param num_columns: Width of the image.
    @return Scaling factor for the vignette effect.
*/
double calculate_vignette_scaling_factor(int row, int col, int num_rows, int num_columns)
{
    // Distance from pixel to the center of the image
    double distance = sqrt(pow(row - num_rows / 2.0, 2) + pow(col - num_columns / 2.0, 2));
    
    // Calculate scaling factor using the above distance
    double scaling_factor = (num_rows - distance) / num_rows;
    return scaling_factor;
}


//***************************************************************************************************//
//                              EDITING PROCESS FUNCTIONS                                           //

// PROCESS 1
/**
    Applies the vignette effect to an image by enhancing its center 
    and fading the edges to darken its corners.
    This effect is achieved by adjusting each pixel's RGB values 
    based on its distance from the center.

    @param image: The original image as a 2D vector of Pixel structs.
    @returns A modified image with the vignette effect applied.
*/
vector<vector<Pixel>> process1(const vector<vector<Pixel>>& image)
{
    // Get image dimensions
    pair<int, int> dimensions = get_image_dimensions(image);
    int num_rows = dimensions.first;
    int num_columns = dimensions.second;
    
    // Initialize a new image with same size
    vector<vector<Pixel>> new_image = initialize_new_image(num_rows, num_columns);
    
    // Process each pixel in the original image
    for (int row = 0; row < num_rows; row++)
    {
        for (int col = 0; col < num_columns; col++)
        {
            // Extract RGB values for current pixel
            tuple<int, int, int> rgb_values = get_pixel_rgb(image, row, col);
            int red_value = get<0>(rgb_values);
            int green_value = get<1>(rgb_values);
            int blue_value = get<2>(rgb_values);
            
            // Calculate new RGB values based on scaling factor
            double scaling_factor = calculate_vignette_scaling_factor(row, col, num_rows, num_columns);
            int new_red = max(0, min(255, static_cast<int>(red_value * scaling_factor)));
            int new_green = max(0, min(255, static_cast<int>(green_value * scaling_factor)));
            int new_blue = max(0, min(255, static_cast<int>(blue_value * scaling_factor)));
            
            // Store the processed pixel in the new image
            store_pixel(new_image, row, col, new_red, new_green, new_blue);
        }
    }
    return new_image;
}
 
    
// PROCESS 2
/**
    The Clarendon effect boosts the contrast in an image.
    Darker pixels become even darker, and lighter pixels become lighter, producing a vivid look.
    This effect is realized by adjusting the RGB values of each pixel depending on its original intensity.

    @param image: The original image represented as a 2D vector of Pixel structs.
    @param scaling_factor: Multiplier to adjust the contrast intensity.
    @returns A modified image with the Clarendon effect.
*/
vector<vector<Pixel>> process2(const vector<vector<Pixel>>& image, double scaling_factor)
{    
    // Fetch the image dimensions
    pair<int, int> dimensions = get_image_dimensions(image);
    int num_rows = dimensions.first;
    int num_columns = dimensions.second;
    
    // Prepare a new image of the same dimensions
    vector<vector<Pixel>> new_image = initialize_new_image(num_rows, num_columns);
    
    // Traverse through each pixel of the original image
    for (int row = 0; row < num_rows; row++)
    {
        for (int col = 0; col < num_columns; col++)
        {
            // Extract the RGB values of the current pixel
            tuple<int, int, int> rgb_values = get_pixel_rgb(image, row, col);
            int red_value = get<0>(rgb_values);
            int green_value = get<1>(rgb_values);
            int blue_value = get<2>(rgb_values);
            
            // Determine the grayscale intensity of the pixel (average of RGB)
            int average_value = round(grey_value(red_value, green_value, blue_value));
            
            int new_red, new_green, new_blue; // To hold new RGB values
            
            // Adjust RGB values based on the pixel's intensity
            if (average_value >= 170)  // Lighter pixels
            {
                new_red = min(255, static_cast<int>(255 - (255 - red_value) * scaling_factor));
                new_green = min(255, static_cast<int>(255 - (255 - green_value) * scaling_factor));
                new_blue = min(255, static_cast<int>(255 - (255 - blue_value) * scaling_factor));
            }
            else if (average_value < 90)  // Darker pixels
            {
                new_red = max(0, static_cast<int>(red_value * scaling_factor));
                new_green = max(0, static_cast<int>(green_value * scaling_factor));
                new_blue = max(0, static_cast<int>(blue_value * scaling_factor));
            }
            else  // Medium intensity pixels remain unchanged
            {
                new_red = red_value;
                new_green = green_value;
                new_blue = blue_value;
            }
            
            // Insert the modified pixel into the new image
            store_pixel(new_image, row, col, new_red, new_green, new_blue);
        }
    }

    return new_image;  // Return the image with applied Clarendon effect
}    

    
// PROCESS 3
/**
    Applies the Greyscale effect to an image by converting each pixel's RGB values 
    into their average. This turns the image into shades of grey while maintaining 
    the original intensity of each pixel.

    @param image: The original image represented as a 2D vector of Pixel structs.
    @returns An image transformed into greyscale.
*/
vector<vector<Pixel>> process3(const vector<vector<Pixel>>& image)
{
    // Fetch the image dimensions
    pair<int, int> dimensions = get_image_dimensions(image);
    int num_rows = dimensions.first;
    int num_columns = dimensions.second;
    
    // Prepare a new image of the same dimensions
    vector<vector<Pixel>> new_image = initialize_new_image(num_rows, num_columns);
    
    // Traverse through each pixel of the original image
    for (int row = 0; row < num_rows; row++)
    {
        for (int col = 0; col < num_columns; col++)
        {
            // Extract the RGB values of the current pixel
            tuple<int, int, int> rgb_values = get_pixel_rgb(image, row, col);
            int red_value = get<0>(rgb_values);
            int green_value = get<1>(rgb_values);
            int blue_value = get<2>(rgb_values);
            
            // Determine the grayscale value (average of RGB)
            int average_value = round(grey_value(red_value, green_value, blue_value));
            
            // Convert pixel to greyscale by setting its RGB values to the average value
            int new_red = average_value;
            int new_green = average_value;
            int new_blue = average_value;
            
            // Insert the greyscale pixel into the new image
            store_pixel(new_image, row, col, new_red, new_green, new_blue);
        }
    }

    return new_image;  // Return the image in greyscale
}    

    
// PROCESS 4
/**
    Applies the Rotate-by-90 effect to an image by repositioning each pixel such that 
    the image is rotated clockwise by 90 degrees. The RGB values of each pixel remain the same.

    @param image: The original image represented as a 2D vector of Pixel structs.
    @returns An image that has been rotated 90 degrees clockwise.
*/
vector<vector<Pixel>> process4(const vector<vector<Pixel>>& image)
{
    // Fetch the image dimensions
    pair<int, int> dimensions = get_image_dimensions(image);
    int num_rows = dimensions.first;
    int num_columns = dimensions.second;
    
    // Initialize a new image with swapped dimensions due to the 90-degree rotation
    vector<vector<Pixel>> new_image = initialize_new_image(num_columns, num_rows);
    
    // Traverse through each pixel of the original image
    for (int row = 0; row < num_rows; row++)
    {
        for (int col = 0; col < num_columns; col++)
        {
            // Extract the RGB values of the current pixel
            tuple<int, int, int> rgb_values = get_pixel_rgb(image, row, col);
            int red_value = get<0>(rgb_values);
            int green_value = get<1>(rgb_values);
            int blue_value = get<2>(rgb_values);        
            
            // Place the pixel in its new position in the rotated image
            // Here, columns become rows, and rows are inverted to become columns
            store_pixel(new_image, col, (num_rows - 1) - row, red_value, green_value, blue_value);
        }
    }

    return new_image;  // Return the rotated image
}

    
// PROCESS 5
/**
    Rotates an image by multiples of 90 degrees based on the user's input.
    
    @param image: The original image represented as a 2D vector of Pixel structs.
    @param number: An integer value corresponding to the number of 90-degree rotations desired.
    @returns A modified image that has been rotated by the specified multiple of 90 degrees.
*/
vector<vector<Pixel>> process5(const vector<vector<Pixel>>& image, int number)
{
    switch(number % 4)  // As we are rotating in 90 degree increments, taking modulo 4 will give us the effective rotations
    {
        case 0:
            // No rotation needed
            return image;
        case 1:
            // 90-degree rotation
            return process4(image);
        case 2:
            // 180-degree rotation (two 90-degree rotations)
            return process4(process4(image));
        case 3:
            // 270-degree rotation (three 90-degree rotations)
            return process4(process4(process4(image)));
    }
    // This part shouldn't execute
    cout << "Unexpected rotation value. Returning original image." << endl;
    return image;
}

    
// PROCESS 6
/**
    Enlarges an image based on specified x and y scale factors. Pixels in the enlarged image 
    are mapped back to their respective positions in the original image.

    @param image: The original image, represented as a 2D vector of Pixel structs.
    @param xscale: Scale factor for the width.
    @param yscale: Scale factor for the height.
    @returns An image enlarged by the given x and y scale factors.
*/
vector<vector<Pixel>> process6(const vector<vector<Pixel>>& image, int xscale, int yscale)
{
    // Obtain the original image's dimensions
    pair<int, int> dimensions = get_image_dimensions(image);
    
    // Calculate new dimensions for the enlarged image
    int new_rows = yscale * dimensions.first;
    int new_cols = xscale * dimensions.second;

    // Set up the enlarged image with the new dimensions
    vector<vector<Pixel>> new_image = initialize_new_image(new_rows, new_cols);
    
    // Iterate through each position in the enlarged image
    for (int row = 0; row < new_rows; row++)
    {
        for (int col = 0; col < new_cols; col++)
        {
            // Retrieve the corresponding pixel's RGB values from the original image
            tuple<int, int, int> rgb_values = get_pixel_rgb(image, row / yscale, col / xscale);
            
            // Fill the enlarged image's pixel with the fetched RGB values
            store_pixel(new_image, row, col, get<0>(rgb_values), get<1>(rgb_values), get<2>(rgb_values));
        }
    }

    return new_image;  // Return the enlarged image
}


// PROCESS 7
/**
    Converts an image to high-contrast black and white using a threshold based on pixel brightness. 
    Pixels with a brightness above the threshold are turned white; those below are turned black.

    @param image: The original image represented as a 2D vector of Pixel structs.
    @returns An image that has been modified for high-contrast black and white effect.
*/
vector<vector<Pixel>> process7(const vector<vector<Pixel>>& image)
{
    // Determine the dimensions of the original image
    pair<int, int> dimensions = get_image_dimensions(image);
    int num_rows = dimensions.first;
    int num_columns = dimensions.second;
    
    // Prepare the modified image with the same dimensions
    vector<vector<Pixel>> new_image = initialize_new_image(num_rows, num_columns);
    
    // Traverse the original image to apply the effect
    for (int row = 0; row < num_rows; row++)
    {
        for (int col = 0; col < num_columns; col++)
        {
            // Fetch the RGB values of the current pixel
            tuple<int, int, int> rgb_values = get_pixel_rgb(image, row, col);
            int average_value = round(grey_value(get<0>(rgb_values), get<1>(rgb_values), get<2>(rgb_values)));
            
            // Determine new RGB values based on the brightness threshold
            int new_red, new_green, new_blue;
            if (average_value >= 128)  // Half of 255 to decide the threshold
            {
                new_red = new_green = new_blue = 255;  // Assign white
            }
            else
            {
                new_red = new_green = new_blue = 0;  // Assign black
            }

            // Update the pixel in the modified image
            store_pixel(new_image, row, col, new_red, new_green, new_blue);
        }
    }

    return new_image;  // Return the processed image
}

    
// PROCESS 8
/**
    Adjusts an image's brightness by increasing pixel RGB values based on a scaling factor, 
    ensuring that no values exceed the maximum permissible limit.

    @param image: The original image represented as a 2D vector of Pixel structs.
    @param scaling_factor: Multiplier to adjust the brightness.
    @returns An image that has been modified to exhibit the lightening effect.
*/
vector<vector<Pixel>> process8(const vector<vector<Pixel>>& image, double scaling_factor)
{
    // Determine the dimensions of the original image
    pair<int, int> dimensions = get_image_dimensions(image);
    int num_rows = dimensions.first;
    int num_columns = dimensions.second;
    
    // Prepare the modified image with the same dimensions
    vector<vector<Pixel>> new_image = initialize_new_image(num_rows, num_columns);
    
    // Traverse the original image to apply the lightening effect
    for (int row = 0; row < num_rows; row++)
    {
        for (int col = 0; col < num_columns; col++)
        {
            // Fetch the RGB values of the current pixel
            tuple<int, int, int> rgb_values = get_pixel_rgb(image, row, col);
            
            // Adjust RGB values based on the scaling factor and confine within the permissible range
            int new_red = min(255, max(0, 255 - static_cast<int>(round((255 - get<0>(rgb_values)) * scaling_factor))));
            int new_green = min(255, max(0, 255 - static_cast<int>(round((255 - get<1>(rgb_values)) * scaling_factor))));
            int new_blue = min(255, max(0, 255 - static_cast<int>(round((255 - get<2>(rgb_values)) * scaling_factor))));

            
            // Store the adjusted pixel in the modified image
            store_pixel(new_image, row, col, new_red, new_green, new_blue);
        }
    }
    return new_image;  // Return the lightened image
}


// PROCESS 9
/**
    Applies a "Darkening" effect on an image by reducing its brightness using a scaling factor. 
    This process ensures that RGB values never fall below the minimum limit.

    @param image: Original image in the form of a 2D vector of Pixel structs.
    @param scaling_factor: Factor to reduce brightness.
    @returns Modified image with the darkening effect.
*/
vector<vector<Pixel>> process9(const vector<vector<Pixel>>& image, double scaling_factor)
{
    // Determine image dimensions
    pair<int, int> dimensions = get_image_dimensions(image);
    int num_rows = dimensions.first;
    int num_columns = dimensions.second;
    
    // Create a new image of same dimensions
    vector<vector<Pixel>> new_image = initialize_new_image(num_rows, num_columns);
    
    // Iterate over each pixel of the original image
    for (int row = 0; row < num_rows; row++)
    {
        for (int col = 0; col < num_columns; col++)
        {
            // Extract the RGB values of the current pixel
            tuple<int, int, int> rgb_values = get_pixel_rgb(image, row, col);
            
            // Compute the adjusted RGB values using the scaling factor and confine them within the permissible range
            int new_red = min(255, max(0, static_cast<int>(get<0>(rgb_values) * scaling_factor)));
            int new_green = min(255, max(0, static_cast<int>(get<1>(rgb_values) * scaling_factor)));
            int new_blue = min(255, max(0, static_cast<int>(get<2>(rgb_values) * scaling_factor)));
            
            // Update the pixel in the new image
            store_pixel(new_image, row, col, new_red, new_green, new_blue);
        }
    }

    return new_image;  // Return the darkened image
}


/**
    Convert the image to emphasize only the colors: Black, White, Red, Blue, and Green.

    @param the original image, in the form of a 2D vector of Pixel structs.
    @returns a modified image that emphasizes the Black, White, Red, Blue, and Green colors.
*/
vector<vector<Pixel>> process10(const vector<vector<Pixel>>& image)
{
    // Get the image dimensions
    pair<int, int> dimensions = get_image_dimensions(image);
    int num_rows = dimensions.first;
    int num_columns = dimensions.second;
    
    // Initialize an image to store the modifications
    vector<vector<Pixel>> new_image = initialize_new_image(num_rows, num_columns);
    
    // Declare new RGB values in advance
    int new_red, new_green, new_blue; 

    // Iterate through each pixel of the input image
    for (int row = 0; row < num_rows; row++)
    {
        for (int col = 0; col < num_columns; col++)
        {
            // Extract the current pixel's RGB values
            tuple<int, int, int> rgb_values = get_pixel_rgb(image, row, col);
            int red_value = get<0>(rgb_values);
            int green_value = get<1>(rgb_values);
            int blue_value = get<2>(rgb_values);
            
            // Determine dominant color and adjust pixel accordingly
            int max_color = max(red_value, max(green_value, blue_value));
            if ((red_value + green_value + blue_value) >= 550)   // White
            {
                new_red = 255;
                new_green = 255;
                new_blue = 255;
            }
            else if ((red_value + green_value + blue_value) <= 150)  // Black
            {
                new_red = 0;
                new_green = 0;
                new_blue = 0;
            }
            else if (max_color == red_value)  // Red
            {
                new_red = 255;
                new_green = 0;
                new_blue = 0;
            }
            else if (max_color == green_value)  // Green
            {
                new_red = 0;
                new_green = 255;
                new_blue = 0;
            }
            else  // Blue
            {
                new_red = 0;
                new_green = 0;
                new_blue = 255;
            }

            // Store the modified pixel in the new image
            store_pixel(new_image, row, col, new_red, new_green, new_blue);
        }
    }

    return new_image;
}

//***************************************************************************************************//

int main()
{
    bool done = false; // controls main while loop
    bool processed = false; // for if image processing was successful

    // Print welcome message
    cout << endl;
    cout << "Welcome to my CSPB 1300 Image Processing Application" << endl << endl;
    
    while (!done) 
    {
        // Reset processed indicator at the start of each iteration
        processed = false;
        string input_filename, output_filename; // to store input and output filenames
        
        vector<vector<Pixel>> input_image, output_image; // to store input and out images
        
        // Get input filename from user. Potential error handled in get_filename function
        input_filename = get_filename("Enter input BMP filename (or 'q' to quit): \n");
        if (input_filename == "q") {return 0; }
        
        // Read in BMP image file into a 2D vector
        input_image = read_image(input_filename);
        
        // If file is empty or doesn't exist in directory, don't end program. Let user retry
        while (input_image.empty()) 
        {
            cout << "Error: Unable to open the file or the file doesn't exist. Please enter a valid filename.\n";
            input_filename = get_filename("Enter input BMP filename (or 'q' to quit): \n");
            if (input_filename == "q") {return 0;}
            input_image = read_image(input_filename);
        }
        
        // Get output filename from user. Potential error handled in get_filename function
        output_filename = get_filename("Enter output BMP filename (or 'q' to quit): \n");
        if (output_filename == "q") {return 0; }
        
        // To prevent overwriting the input file with the output file
        while (!ensure_unique_output_filename(input_filename, output_filename)) 
        {
            output_filename = get_filename("Enter a different output BMP filename (or 'q' to quit): \n");
            if (output_filename == "q") {return 0;}
        }   

        string string_menu_choice; // to store string user choice
        int choice; // to store int user choice
        
        // Display menu and prompt user for edit type selection
        choice = prompt_and_get_menu_choice(input_filename);
        
        double scaling_factor; // to collect user-supplied scaling_factor
        
        switch (choice) 
        {
            
            case -2: // User wants to exit
                
                done = true;
                break;
                
            case 0: // This will exit the inner loop, going back to image selection menu
                
                cout << "Going back to image selection.\n";
                cout << endl;
                break;

            case 1: // Vignette effect
            
                cout << "Vignette selected\n";
                cout << endl;
                output_image = process1(input_image);
                processed = true;
                break;

            case 2: // Clarendon type effect

                cout << "Clarendon selected\n";
                cout << endl;
                cout << "Please enter a scaling factor between 0.0 and 1.0 \n";
                cout << endl;
                cin >> scaling_factor;
                cin.ignore(numeric_limits<streamsize>::max(), '\n'); // clear input buffer

                // Check if the input is valid and within range
                while (handle_input_error() || scaling_factor < 0.0 || scaling_factor > 1.0) 
                {
                    cout << "Invalid input. Please enter a scaling factor between 0.0 and 1.0: ";
                    cin >> scaling_factor;
                }
                output_image = process2(input_image, scaling_factor);
                processed = true;
                break;

            case 3: // Greyscale effect
                
                cout << "Greyscale selected\n";
                cout << endl;
                output_image = process3(input_image);
                processed = true;
                break;

            case 4: // Rotates by 90 degrees
                
                cout << "Rotate by 90 selected\n";
                cout << endl;
                output_image = process4(input_image);
                processed = true;
                break;

            case 5: // Rotates by 90 degrees n times

                int n;
                cout << "Rotate by 90 n number of times selected\n";
                cout << endl;
                cout << "Please enter number of times you would like to rotate by 90 degrees \n";
                cout << endl;
                cin >> n;
                cin.ignore(numeric_limits<streamsize>::max(), '\n'); // clear input buffer

                // Check if the input is greater than zero)
                while (handle_input_error() || n < 0)
                {
                    cout << "Error. Please enter a valid number of times you would like image to rotate: ";
                    cin >> n;
                }
                output_image = process5(input_image, n);
                processed = true;
                break;

            case 6: // Enlargens image
                
                int x_scale, y_scale;

                cout << "Enlargen selected\n";
                cout << endl;
                cout << "Please enter non-negative integers for x-scale and y-scale separated by just a space: \n";
                cout << endl;
                cin >> x_scale >> y_scale;
                cin.ignore(numeric_limits<streamsize>::max(), '\n'); // clear input buffer

                // Check if the inputs are valid (both should be integers and greater than zero)
                while (handle_input_error() || x_scale < 0 || y_scale < 0) 
                {
                    cout << "Invalid input. Both scales should be non-negative integers. Please enter x-scale and y-scale separated byjust a space: ";
                    cout << endl;
                    cin >> x_scale >> y_scale;
                }
                output_image = process6(input_image, x_scale, y_scale);
                processed = true;
                break;

            case 7:  // High contrast
              
                cout << "High-contrast selected\n";
                cout << endl;
                output_image = process7(input_image);
                processed = true;
                break;

            case 8: // Lightens image
                
                cout << "Lighten selected\n";
                cout << endl;
                cout << "Please enter a scaling factor between 0.0 and 1.0 \n";
                cout << endl;
                cin >> scaling_factor;
                cin.ignore(numeric_limits<streamsize>::max(), '\n'); // clear input buffer

                while (handle_input_error() || scaling_factor < 0.0 || scaling_factor > 1.0) 
                {
                    cout << "Invalid input. Please enter a scaling factor between 0.0 and 1.0: \n";
                    cin >> scaling_factor;
                }
                output_image = process8(input_image, scaling_factor);
                processed = true;
                break;

            case 9: // Darkens image
                
                cout << "Darken selected\n";
                cout << endl;
                cout << "Please enter a scaling factor between 0.0 and 1.0 \n";
                cout << endl;
                cin >> scaling_factor;
                cin.ignore(numeric_limits<streamsize>::max(), '\n'); // clear input buffer

                while (handle_input_error() || scaling_factor < 0.0 || scaling_factor > 1.0) 
                {
                    cout << "Invalid input. Please enter a scaling factor between 0.0 and 1.0: \n";
                    cin >> scaling_factor;
                }
                output_image = process9(input_image, scaling_factor);
                processed = true;
                break;

            case 10: // Convert to only black, white, red, blue, and green
                
                cout << "Black, white, red, blue, and green selected\n";
                cout << endl;
                output_image = process10(input_image);
                processed = true;
                break;

            default:
                cout << "Wrong choice. Please choose a valid option: \n";
         }
        
        // Check if any processing was successful, if so, 
        // write the image, display success, and take user back to start (image selection)
        // Else, take user back to start(image selection) until they quit
        if (processed)
        {
            //Write the resulting 2D vector to a new BMP image file (using write_image function)
            bool success = write_image(output_filename, output_image);

            if (!success)
            {
                cout << "Sorry, image processing failed. Going back to image selection...\n";
            }
            else
            {
                display_success_message(choice);
            }
        }
    }
    return 0;       
}
