from ultralytics import YOLO

if __name__ == '__main__':
    # Create a new YOLO26n-OBB model from scratch
    model = YOLO("./cfg/yolo26-obb.yaml")

    # Train the model on the DOTAv1 dataset
    results = model.train(
        data="./cfg/data_dl1.yaml", 
        epochs=50, 
        imgsz=512,
        project="./runs",           # 主输出目录
        )
