from ultralytics import YOLO
import time
import cv2
import os
import glob
import numpy as np


model = YOLO('./runs/train/weights/best.pt')
# model.to('cpu')

dateRoot = "..\\Data"
resRoot = ".\\res\\yolo"
vecTask = [dateRoot + "\\tra", dateRoot + "\\rot", dateRoot + "\\lig_rect", dateRoot + "\\lig_qb", dateRoot + "\\lig_qbc"]
vecFolderPath = []
for taskPath in vecTask:
    with os.scandir(taskPath + "\\") as entries:
        folderPaths = [entry.name for entry in entries if entry.is_dir()]
        for folderPath in folderPaths:
            vecFolderPath.append(taskPath + "\\" + folderPath)

for folderPath in vecFolderPath:    
    folderItems = folderPath.split("\\")
    resPath = resRoot + "\\" + folderItems[-2]
    if not os.path.exists(resPath):
       os.makedirs(resPath)
    resFilePath = resPath +"\\" + folderItems[-1] + ".txt"
    file = open(resFilePath, 'w', encoding='utf-8')

    imgPaths = glob.glob(folderPath + "\\*.bmp")
    for i, imgPath in enumerate(imgPaths):
        query_image_path = imgPaths[i]
        items1 = imgPaths[i].split("\\")
        fileName = items1[-1]

        image = cv2.imread(query_image_path)
        img_ctr_x = (image.shape[1] - 1) * 0.5
        img_ctr_y = (image.shape[0] - 1) * 0.5

        start_time =  time.perf_counter()
        results = model(image)
        time_res = time.perf_counter() - start_time
        
        wstr = ''
        if len(results) > 0 and results[0].obb is not None and len(results[0].obb) > 0:
            # 获取所有检测结果的置信度
            confs = results[0].obb.conf.cpu().numpy()
          
            # 找到置信度最高的索引
            best_idx = np.argmax(confs)
          
            # 提取置信度最高的检测框信息
            best_box = results[0].obb.xywhr.cpu().numpy()[best_idx]  # (5,)
            best_conf = confs[best_idx]  # 最高置信度

            wstr = fileName + " " + str(50 + best_conf * 50) + " " + str(time_res * 1000) + " " + str(best_box[0] - img_ctr_x) + " " + str(best_box[1] - img_ctr_y) + " " + str(best_box[4] / np.pi * 180.) + "\n"
        else:
            wstr = fileName + " " + str(0.0) + " " + str(time_res * 1000) + " " + str(0.0) + " " + str(0.0) + " " + str(0.0) + "\n"
        file.write(wstr)
    file.close()
