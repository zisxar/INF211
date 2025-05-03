import re

db_product = {}
db_TIN = {}

def isEOF(l):
    if not l:
        raise Exception('EOFOccured')

def skipLinesUntilTheNextReceipt(f):
    line = f.readline().strip()
    isEOF(line)
    while re.match("-{"+re.escape(str(len(line)))+"}", line) == None:
        line = f.readline().strip()
        isEOF(line)
    return line

def readReceipt(f):

    l = list()
    t = tuple()

    # Check the line with the TIN number.
    # Check the arguments, must be two.
    line = f.readline().strip().split()

    isEOF(line)

    if len(line) != 2:
        raise Exception('SecondLineNotTINSize')

    # Check if the line starts with the string 'ΑΦΜ:'.
    line[0] = line[0].strip()
    if line[0] != 'ΑΦΜ:':
        raise Exception('SecondLineNotTINString')

    # Check if the contents of the TIN number is ten digits.
    line[1] = line[1].strip()
    if len(line[1]) != 10 or re.match("[0-9]{10}", line[1]) == None:
        raise Exception('SecondLineNotTINNumber')

    TIN_number = line[1]

    line = f.readline().strip()
    isEOF(line)
    total_value = 0

    while True:
        dash_only = re.match("-{"+re.escape(str(len(line)))+"}", line) != None
        if dash_only:
            raise Exception('incompleteReceipt')

        product_line = line.split(':')
        product_line[0] = product_line[0].strip()
        product_line[1] = product_line[1].strip()
        
        if len(product_line) != 2:
            raise Exception('productLineWrongSize')
        
        if product_line[0] == 'ΣΥΝΟΛΟ':

            product_line[1] = product_line[1].split()
            if len(product_line[1]) != 1:
                raise Exception('sumLineTotalValueWrongSize')

            try:
                product_line[1] = float("{0:.2f}".format(float(product_line[1][0])))
            except ValueError:
                raise Exception('sumLineTotalWrongValue')

            if total_value != product_line[1]:
                raise Exception('sumLineTotalWrongCalculation')

            if not l:
                raise Exception('noProducts')

            break

        value = product_line[1].strip().split()
        
        if len(value) != 3:
            raise Exception('productLineValueWrongSize')

        try:
            quantity = float("{0:.2f}".format(float(value[0]))) # Check for characters
            unit_value = float("{0:.2f}".format(float(value[1])))
            total_unit_value = float("{0:.2f}".format(float(quantity * unit_value)))
            value[2] = float("{0:.2f}".format(float(value[2])))
        except ValueError:
            raise Exception('productLineNotAValue')

        if total_unit_value != value[2]:
            raise Exception('productLineValueWrongCalculation')

        try:
            total_value = float("{0:.2f}".format(float(total_unit_value + total_value)))
        except ValueError:
            raise Exception('productLineNotAValue1')

        product_found = False
        product_line[0] = product_line[0].lower()
        # check if the product already exists in the list of tuples
        if l: # l not empty
            for i in range(0, len(l)):
                if l[i][0] == product_line[0]:
                    new_value = float("{0:.2f}".format(float(value[2] + l[i][1])))
                    l[i] = tuple([product_line[0], new_value])
                    product_found = True
                    break
            if not product_found:
                t = tuple([product_line[0], value[2]])
                l.append(t)
        else: # l is empty
            # initiate tuple
            t = tuple([product_line[0], value[2]])
            l.append(t)

        line = f.readline().strip()
        isEOF(line)
    return line, TIN_number, l # + values + product list


def readNewInputFile():

    file_is_open = False
    file_name = input('Please enter the name of the new input file: ')
    try:
        r1 = open(file_name, 'r', encoding='utf-8')
        file_is_open = True
    except FileNotFoundError:
        # go to initial menu
        file_is_open = False
        printMenu()


    if file_is_open:


        line = r1.readline().strip()
        dash_only = re.match("-{"+re.escape(str(len(line)))+"}", line) != None
        if not dash_only:
            try:
                line = skipLinesUntilTheNextReceipt(r1)
                dash_only = True
            except Exception as e:
                if e.args[0] == 'EOFOccured':
                    dash_only = False
        tmp_list_TIN = []

        while dash_only :
            try:
                excepted = False
                line, TIN_number, tmp_list_TIN = readReceipt(r1)
                TIN_number = 'tin'+TIN_number
            except Exception as e:
                excepted = True
                if e.args[0] == 'incompleteReceipt':
                    dash_only = True
                    pass
                if e.args[0] == 'EOFOccured':
                    break


            line = r1.readline().strip()
            if not line:
                break

            dash_only = re.match("-{"+re.escape(str(len(line)))+"}", line) != None
            if dash_only and not excepted:
                
                # Database for product
                if db_product: # db is not empty
                    for product in tmp_list_TIN:
                        if product[0] in db_product: # product exists in the database
                            product_found = False
                            i = 0
                            for tin_num in db_product[product[0]]:
                                if tin_num[0] == TIN_number: # find the TIN number in the list of the database
                                    # update the value
                                    product_found = True
                                    tmp_value0 = float(tin_num[1])
                                    tmp_value1 = float(product[1])
                                    new_value = float("{0:.2f}".format(float(tmp_value0 + tmp_value1)))
                                    db_product[product[0]][i] = tuple([TIN_number, new_value])
                                    break

                                i += 1
                            if not product_found: # insert the product in the list
                                db_product[product[0]].append(tuple([TIN_number, product[1]]))
                                product_found = False

                            # sort the list
                            db_product[product[0]].sort()
                                    
                        else: # insert the new key
                            db_product[product[0]] = [tuple([TIN_number, product[1]])]

                else: # db is empty
                    for product in tmp_list_TIN:
                        db_product[product[0]] = [tuple([TIN_number, product[1]])]
                        

                # Database for TIN number
                if db_TIN: # db is not empty
                    if TIN_number in db_TIN: # TIN number exists in the database
                        for tmp_product in tmp_list_TIN:
                            product_found = False
                            i = 0
                            for db_product_value in db_TIN[TIN_number]:
                                if tmp_product[0] == db_product_value[0]:
                                    # update value
                                    product_found = True
                                    tmp_value0 = float(tmp_product[1])
                                    tmp_value1 = float(db_product_value[1])
                                    new_value = float("{0:.2f}".format(float(tmp_value0 + tmp_value1)))
                                    db_TIN[TIN_number][i] = tuple([tmp_product[0], new_value])
                                    break

                                i += 1
                            if not product_found:
                                db_TIN[TIN_number].append(tmp_product)
                                product_found = False

                            # sort the list
                            db_TIN[TIN_number].sort()

                    else: # insert the new key
                        db_TIN[TIN_number] = tmp_list_TIN

                else: # db is empty
                    tmp_list_TIN.sort()
                    db_TIN[TIN_number] = tmp_list_TIN

            else:# more products after the sum, decline the receipt
                try:
                    line = skipLinesUntilTheNextReceipt(r1)
                    dash_only = True
                except Exception as e:
                    if e.args[0] == 'EOFOccured':
                        break

        r1.close()

def productQuery():
    product_name = input('Please enter the name of the product: ')

    product_name = product_name.lower()
    if product_name in db_product:
        for tin_num in db_product[product_name]:
            print(tin_num[0][3:], tin_num[1])

def TINnumberQuery():
    TIN_number = input('Please enter the TIN number: ')
    TIN_number = 'tin' + TIN_number

    if TIN_number in db_TIN:
        for product_value in db_TIN[TIN_number]:
            print(product_value[0].upper(), product_value[1])

def printMenu():

    try:
        answer = int(input('Give your preference: (1: read new input file, 2: print statistics for a specific product, 3: print statistics for a specific AFM, 4: exit the program)'))
    except Exception:
        answer = None
    if answer == 1:
        readNewInputFile();
    elif answer == 2:
        productQuery();
    elif answer == 3:
        TINnumberQuery();
    elif answer == 4:
        db_TIN.clear()
        db_product.clear()
        exit()

while True :
    printMenu()