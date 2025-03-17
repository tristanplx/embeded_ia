
import tensorflow as tf
from tensorflow import keras
from tensorflow.keras import layers,models
import matplotlib.pyplot as plt
import IPython
import sys, os, array, time
import serial 
import numpy as np
import pandas as pd
from sklearn.model_selection import train_test_split
from tensorflow.keras.layers import Dense, BatchNormalization, Dropout, ReLU, Input
from tensorflow.keras.regularizers import l2
from tensorflow.keras.optimizers import Adam
import tensorflow as tf
from sklearn.metrics import confusion_matrix,classification_report, ConfusionMatrixDisplay
import seaborn as sns
from sklearn.utils.class_weight import compute_class_weight  
from imblearn.over_sampling import SMOTE

#raw data generation
Raw_data = pd.read_excel('./ai4i2020.xlsx')
filtered_data = Raw_data[Raw_data['Machine failure']==1]
filtered_data['No specific data'] = (~(filtered_data['TWF']==1) & ~(filtered_data['HDF']==1) & ~(filtered_data['PWF']==1) & ~(filtered_data['OSF']==1) & ~(filtered_data['RNF']==1) )
Raw_data["Functionnal"] = 1-Raw_data['Machine failure']
data_df = Raw_data.drop(['Type','UDI','Product ID','RNF', 'Machine failure'], axis=1)
data_inputs = data_df.iloc[:,0:5]
data_outputs = data_df.iloc[:,5:]
X_train, X_test, Y_train, Y_test = train_test_split(data_inputs, data_outputs, test_size=0.3, random_state=42)
X_test,X_valid,Y_test,Y_valid = train_test_split(X_test, Y_test, test_size=0.5, random_state=42)
output_labels = data_outputs.columns.tolist()


#data balancing
y_train_labels = Y_train.idxmax(axis=1).astype('category').cat.codes
y_test_labels = Y_test.idxmax(axis=1).astype('category').cat.codes
y_valid_labels = Y_valid.idxmax(axis=1).astype('category').cat.codes
#using SMOTE technique
#auto balancing classes according to the biggest one
smote = SMOTE(sampling_strategy='auto', random_state=42)  
X_train_resampled, Y_train_resampled = smote.fit_resample(X_train, y_train_labels)
X_test_resampled, Y_test_resampled = smote.fit_resample(X_test, y_test_labels)
X_valid_resampled, Y_valid_resampled = smote.fit_resample(X_valid, y_valid_labels)
#back to One hot encoding
Y_train_resampled = pd.get_dummies(Y_train_resampled) 
Y_test_resampled = pd.get_dummies(Y_test_resampled) 
Y_valid_resampled = pd.get_dummies(Y_valid_resampled) 
#class weights
y_train_labels = Y_train_resampled.idxmax(axis=1).astype('category').cat.codes
#Isolating labels
unique_classes = np.unique(y_train_labels)
#Class wheight computing
class_weights = compute_class_weight(
    class_weight='balanced',
    classes=unique_classes,
    y=y_train_labels
)
#Wheight to dico
class_weight_dict = {int(cls): float(weight) for cls, weight in zip(unique_classes, class_weights)}


#model
inputs = Input(shape=(5,))  

# Dense layers with L2 regularization, ReLu activation and dropout rate of 25%


x = Dense(512, kernel_regularizer=l2(0.001))(inputs)
x = BatchNormalization()(x)
x = ReLU()(x)
x = Dropout(0.25)(x)


x = Dense(256, kernel_regularizer=l2(0.001))(x)
x = BatchNormalization()(x)
x = ReLU()(x)
x = Dropout(0.25)(x)


x = Dense(128, kernel_regularizer=l2(0.001))(x)
x = BatchNormalization()(x)
x = ReLU()(x)
x = Dropout(0.25)(x)


x = Dense(64, kernel_regularizer=l2(0.001))(x)
x = BatchNormalization()(x)
x = ReLU()(x)
x = Dropout(0.25)(x)


x = Dense(32, kernel_regularizer=l2(0.001))(x)
x = BatchNormalization()(x)
x = ReLU()(x)
x = Dropout(0.25)(x)


outputs = Dense(5, activation='softmax')(x)
#Model defintion
weighted = tf.keras.Model(inputs=inputs, outputs=outputs, name="unbalanced")
weighted.summary()
weighted.compile(optimizer=Adam(learning_rate=0.0001),
                   loss='categorical_crossentropy',
                   metrics=['accuracy'])
#Training parameters
nb_epochs = 50
batch_size = 120
#Train the model
training_history_weighted = weighted.fit(X_train_resampled, Y_train_resampled,class_weight=class_weight_dict,validation_data=(X_valid_resampled, Y_valid_resampled),epochs=nb_epochs,batch_size=batch_size)


np.save('numpy_data/X_train.npy',X_train_resampled)
np.save('numpy_data/X_test.npy',X_test_resampled)
np.save('numpy_data/X_valid.npy',X_valid_resampled)
np.save('numpy_data/Y_train.npy',Y_train_resampled)
np.save('numpy_data/Y_test.npy',Y_test_resampled)
np.save('numpy_data/Y_valid.npy',Y_valid_resampled)
weighted.save('Panhelleux_Rabier_balanced_model.h5')

