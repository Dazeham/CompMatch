from mmdet.apis import init_detector, inference_detector
import time
import cv2
import os
import glob
import numpy as np


config_file = '/workspace/ai4rs/projects/rotated_rtdetr/configs/o2_rtdetr_r18vd_2ab4_50e_cm.py'
checkpoint_file = '/workspace/ai4rs/work_dir/train/o2_r18/epoch_50.pth'
model = init_detector(
    config_file,
    checkpoint_file,
    # device='cuda:0'
    device='cpu'
)

dateRoot = "/mnt/g/data_cm/Data"
resRoot = "/workspace/ai4rs/work_dir/test/o2_r18_cpu"
vecTask = [dateRoot + "/tra", dateRoot + "/rot", dateRoot + "/lig_rect", dateRoot + "/lig_qb"]
vecFolderPath = []
for taskPath in vecTask:
    with os.scandir(taskPath + "/") as entries:
        folderPaths = [entry.name for entry in entries if entry.is_dir()]
        for folderPath in folderPaths:
            vecFolderPath.append(taskPath + "/" + folderPath)

for folderPath in vecFolderPath:    
    folderItems = folderPath.split("/")
    resPath = resRoot + "/" + folderItems[-2]
    if not os.path.exists(resPath):
       os.makedirs(resPath)
    resFilePath = resPath +"/" + folderItems[-1] + ".txt"
    file = open(resFilePath, 'w', encoding='utf-8')

    imgPaths = glob.glob(folderPath + "/*.bmp")
    for i, imgPath in enumerate(imgPaths):
        query_image_path = imgPaths[i]
        items1 = imgPaths[i].split("/")
        fileName = items1[-1]

        # image = cv2.imread(query_image_path)
        # img_ctr_x = (image.shape[1] - 1) * 0.5
        # img_ctr_y = (image.shape[0] - 1) * 0.5
        img_ctr_x = (512 - 1) * 0.5
        img_ctr_y = (512 - 1) * 0.5

        start_time =  time.perf_counter()
        result = inference_detector(
            model,
            query_image_path
        )
        time_res = time.perf_counter() - start_time
        
        wstr = ''
        pred = result.pred_instances
        scores = pred.scores.cpu().numpy()
        bboxes = pred.bboxes.cpu().numpy()
        best_idx = np.argmax(scores)
        best_box = bboxes[best_idx]  # (5,)
        best_conf = scores[best_idx]  # Highest confidence

        if best_conf > 0.5:
            ang = best_box[4] / np.pi * 180.
            if ang > 90:
                ang -= 180
            elif ang <-90:
                ang += 180
            wstr = fileName + " " + str(50 + best_conf * 50) + " " + str(time_res * 1000) + " " + str(best_box[0] - img_ctr_x) + " " + str(best_box[1] - img_ctr_y) + " " + str(ang) + "\n"
        else:
            wstr = fileName + " " + str(0.0) + " " + str(time_res * 1000) + " " + str(0.0) + " " + str(0.0) + " " + str(0.0) + "\n"
        file.write(wstr)
    file.close()
