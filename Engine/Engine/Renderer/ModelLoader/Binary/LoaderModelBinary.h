#pragma once

struct DefaultVertex;
class BinaryWriter;

namespace Engine
{
	class LoaderModel;
	class LoaderMesh;

	class LoaderModelBinary
	{
	public:
		struct Header
		{
			uint8_t myMagic[3] = { 'G', '6', 'M' };

			uint8_t version = 1;

			Mat4f myGlobalInverseTransform;
			unsigned int myBonesCount = 0;

			Vec3f myBoundingSphereCenter;
			float myBoundingSphereRadius = 0.f;
		};

		struct BoneToName
		{
			// TODO: Consider removing the buffer and simlpy reading size and then the pointer like in Mesh
			// otherwise wastes too much space
			char myName[260] = {};
			uint32_t myIndex = 0;
		};

		struct Mesh
		{
			const char* myMeshName = nullptr;
			const char* myAlbedoTexturePath = nullptr;
			const char* myNormalTexturePath = nullptr;
			const char* myMaterialTexturePath = nullptr;

			int myVerticesStartIndex = 0;
			int myVerticesCount = 0;

			int myIndicesStartIndex = 0;
			int myIndicesCount = 0;

			void Deserialize(BinaryReader& aBinaryReader);
		};

	public:
		// Used to convert a FBX to our own format
		static std::vector<uint8_t> Serialize(const LoaderModel& aLoaderModel);

		bool Deserialize(const Path& aPath);

		const Header* GetHeader() const;

		const DefaultVertex* GetVertices() const;
		int GetVerticesCount() const;

		const unsigned int* GetIndices() const;
		int GetIndicesCount() const;

		const std::vector<Mesh>& GetMeshes() const;

		const Mat4f* GetBoneOffsets() const;
		int GetBoneOffsetsCount() const;

		const BoneToName* GetBoneToNames() const;
		int GetBoneToNamesCount() const;

	private:
		static void WriteMeshes(BinaryWriter& aBinaryStream, const LoaderModel& aLoaderModel);
		static void WriteBoneOffsets(BinaryWriter& aBinaryStream, const LoaderModel& aLoaderModel);
		static void WriteBoneToName(BinaryWriter& aBinaryStream, const LoaderModel& aLoaderModel);
		static void WriteVertices(BinaryWriter& aBinaryStream, const LoaderModel& aLoaderModel);
		static void WriteIndices(BinaryWriter& aBinaryStream, const LoaderModel& aLoaderModel);
		static void CalculateBoundingSphere(const LoaderModel& aLoaderModel, Header &aHeader);

		void ReadVertices();
		void ReadIndices();
		void ReadMeshes();
		void ReadBoneOffsets();
		void ReadBoneToName();

	private:
		Header* myHeader = nullptr;

		DefaultVertex* myVertices = nullptr;
		int myVerticesCount = 0;

		unsigned int* myIndices = nullptr;
		int myIndicesCount = 0;

		std::vector<Mesh> myMeshes;

		Mat4f* myBoneOffsets = nullptr;
		int myBoneOffsetsCount = 0;

		BoneToName* myBoneToNames = nullptr;
		int myBoneToNamesCount = 0;

		BinaryReader myBinaryStream;
	};
}